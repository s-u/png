#include <stdio.h>
#include <png.h>

#include <Rinternals.h>

typedef struct write_job {
    FILE *f;
    int ptr, len;
    char *data;
    SEXP rvlist, rvtail;
    int rvlen;
} write_job_t;

/* default size of a raw vector chunk when collecting the image result */
#define INIT_SIZE (1024*256)

static void user_error_fn(png_structp png_ptr, png_const_charp error_msg) {
    write_job_t *rj = (write_job_t*)png_get_error_ptr(png_ptr);
    if (rj->f) fclose(rj->f);
    Rf_error("libpng error: %s", error_msg);
}

static void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg) {
    Rf_warning("libpng warning: %s", warning_msg);
}

static void user_write_data(png_structp png_ptr, png_bytep data, png_size_t length) {
    write_job_t *rj = (write_job_t*) png_get_io_ptr(png_ptr);
    png_size_t to_write = length;
    while (length) { /* use iteration instead of recursion */
	if (to_write > (rj->len - rj->ptr))
	    to_write = (rj->len - rj->ptr);
	if (to_write > 0) {
	    memcpy(rj->data + rj->ptr, data, to_write);
	    rj->ptr += to_write;
	    length -= to_write;
	    data += to_write;
	    rj->rvlen += to_write;
	}
	if (length) { /* more to go -- need next buffer */
	    SEXP rv = allocVector(RAWSXP, INIT_SIZE);
	    SETCDR(rj->rvtail, CONS(rv, R_NilValue));
	    rj->rvtail = CDR(rj->rvtail);
	    rj->len = LENGTH(rv);
	    rj->data = (char*) RAW(rv);
	    rj->ptr = 0;
	    to_write = length;
	}
    }
}

static void user_flush_data(png_structp png_ptr) {
}

#if USE_R_MALLOC
static png_voidp malloc_fn(png_structp png_ptr, png_alloc_size_t size) {
    return (png_voidp) R_alloc(1, size);
}

static void free_fn(png_structp png_ptr, png_voidp ptr) {
    /* this is a no-op because R releases the memory at the end of the call */
}
#endif

SEXP write_png(SEXP image, SEXP sFn) {
    SEXP res = R_NilValue, dims;
    const char *fn;
    int planes = 1, width, height;
    FILE *f;
    write_job_t rj;
    png_structp png_ptr;
    png_infop info_ptr;
    
    if (TYPEOF(image) != REALSXP)
	Rf_error("image must be a matrix or array of real numbers");
    
    dims = Rf_getAttrib(image, R_DimSymbol);
    if (dims == R_NilValue || TYPEOF(dims) != INTSXP || LENGTH(dims) < 2 || LENGTH(dims) > 3)
	Rf_error("image must be a matrix or an array of two or three dimensions");

    if (LENGTH(dims) == 3)
	planes = INTEGER(dims)[2];
    if (planes < 1 || planes > 4)
	Rf_error("image must have either 1 (grayscale), 2 (GA), 3 (RGB) or 4 (RGBA) planes");

    width = INTEGER(dims)[1];
    height = INTEGER(dims)[0];
    
    if (TYPEOF(sFn) == RAWSXP) {
	SEXP rv = allocVector(RAWSXP, INIT_SIZE);
	rj.rvtail = rj.rvlist = PROTECT(CONS(rv, R_NilValue));
	rj.data = (char*) RAW(rv);
	rj.len = LENGTH(rv);
	rj.ptr = 0;
	rj.rvlen = 0;
	rj.f = f = 0;
    } else {
	if (TYPEOF(sFn) != STRSXP || LENGTH(sFn) < 1) Rf_error("invalid filename");
	fn = CHAR(STRING_ELT(sFn, 0));
	f = fopen(fn, "wb");
	if (!f) Rf_error("unable to create %s", fn);
	rj.f = f;
    }

    /* use our own error hanlding code and pass the fp so it can be closed on error */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)&rj, user_error_fn, user_warning_fn);
    if (!png_ptr) {
	if (f) fclose(f);
	Rf_error("unable to initialize libpng");
    }
    
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
	if (f) fclose(f);
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	Rf_error("unable to initialize libpng");
    }
    
    if (f)
	png_init_io(png_ptr, f);
    else
	png_set_write_fn(png_ptr, (voidp) &rj, user_write_data, user_flush_data);

    png_set_IHDR(png_ptr, info_ptr, width, height, 8,
		 (planes == 1) ? PNG_COLOR_TYPE_GRAY : ((planes == 2) ? PNG_COLOR_TYPE_GRAY_ALPHA : ((planes == 3) ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA)),
		 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    {
	int rowbytes = width * planes, i;
	png_bytepp row_pointers;
	png_bytep  flat_rows;
	
	row_pointers = (png_bytepp) R_alloc(height, sizeof(png_bytep));
	flat_rows = (png_bytep) R_alloc(height, width * planes);
	for(i = 0; i < height; i++)
	    row_pointers[i] = flat_rows + (i * width * planes);
	
	{
	    int x, y, p, pln = rowbytes / width, pls = width * height;
	    double * data;
	    data = REAL(image);
	    for(y = 0; y < height; y++)
		for (x = 0; x < width; x++)
		    for (p = 0; p < pln; p++) {
			double v = data[y + x * height + p * pls];
			if (v < 0) v = 0;
			if (v > 255.0) v = 1.0;
			row_pointers[y][x * pln + p] = (unsigned char)(v * 255.0);
		    }
	}

    	png_set_rows(png_ptr, info_ptr, row_pointers);
    }

    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    png_destroy_write_struct(&png_ptr, &info_ptr);

    if (f) { /* if it is a file, just return */
	fclose(f);
	return R_NilValue;
    }
    
    /* otherwise collect the vector blocks into one vector */
    res = allocVector(RAWSXP, rj.rvlen);
    {
	int to_go = rj.rvlen;
	unsigned char *data = RAW(res);
	while (to_go && rj.rvlist != R_NilValue) {
	    SEXP ve = CAR(rj.rvlist);
	    int this_len = (to_go > LENGTH(ve)) ? LENGTH(ve) : to_go;
	    memcpy(data, RAW(ve), this_len);
	    to_go -= this_len;
	    data += this_len;
	    rj.rvlist = CDR(rj.rvlist);
	}
    }
    
    UNPROTECT(1);
    return res;
}
