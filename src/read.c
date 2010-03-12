#include <stdio.h>
#include <png.h>

#include <Rinternals.h>

typedef struct read_job {
    FILE *f;
    int ptr, len;
    char *data;
} read_job_t;

static void user_error_fn(png_structp png_ptr, png_const_charp error_msg) {
    read_job_t *rj = (read_job_t*)png_get_error_ptr(png_ptr);
    if (rj->f) fclose(rj->f);
    Rf_error("libpng error: %s", error_msg);
}

static void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg) {
    Rf_warning("libpng warning: %s", warning_msg);
}

static void user_read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
    read_job_t *rj = (read_job_t*) png_get_io_ptr(png_ptr);
    png_size_t to_read = length;
    if (to_read > (rj->len - rj->ptr))
	to_read = (rj->len - rj->ptr);
    if (to_read > 0) {
	memcpy(data, rj->data + rj->ptr, to_read);
	rj->ptr += to_read;
    }
    if (to_read < length)
	memset(data + length - to_read, 0, length - to_read);
}

#if USE_R_MALLOC
static png_voidp malloc_fn(png_structp png_ptr, png_alloc_size_t size) {
    return (png_voidp) R_alloc(1, size);
}

static void free_fn(png_structp png_ptr, png_voidp ptr) {
    /* this is a no-op because R releases the memory at the end of the call */
}
#endif

SEXP read_png(SEXP sFn) {
    SEXP res = R_NilValue;
    const char *fn;
    char header[8];
    FILE *f;
    read_job_t rj;
    png_structp png_ptr;
    png_infop info_ptr;
    
    if (TYPEOF(sFn) == RAWSXP) {
	rj.data = RAW(sFn);
	rj.len = LENGTH(sFn);
	rj.ptr = 0;
	rj.f = f = 0;
    } else {
	if (TYPEOF(sFn) != STRSXP || LENGTH(sFn) < 1) Rf_error("invalid filename");
	fn = CHAR(STRING_ELT(sFn, 0));
	f = fopen(fn, "rb");
	if (!f) Rf_error("unable to open %s", fn);
	if (fread(header, 1, 8, f) < 1 || png_sig_cmp(header, 0, 8)) {
	    fclose(f);
	    Rf_error("file is not in PNG format");
	}
	rj.f = f;
    }

    /* use our own error hanlding code and pass the fp so it can be closed on error */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)&rj, user_error_fn, user_warning_fn);
    if (!png_ptr) {
	if (f) fclose(f);
	Rf_error("unable to initialize libpng");
    }
    
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
	if (f) fclose(f);
	png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
	Rf_error("unable to initialize libpng");
    }
    
    if (f) {
	png_init_io(png_ptr, f);
	png_set_sig_bytes(png_ptr, 8);
    } else
	png_set_read_fn(png_ptr, (voidp) &rj, user_read_data);

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_EXPAND, NULL);
    {
	png_uint_32 width, height;
	SEXP dim;
	int bit_depth, color_type, interlace_type, compression_type, filter_method, rowbytes;
	png_get_IHDR(png_ptr, info_ptr, &width, &height,
		     &bit_depth, &color_type, &interlace_type,
		     &compression_type, &filter_method);
	rowbytes = png_get_rowbytes(png_ptr, info_ptr);
#if 0
	Rprintf("%d x %d [%d], %d bytes, 0x%x, %d, %d\n", (int) width, (int) height, bit_depth, rowbytes,
		color_type, interlace_type, compression_type, filter_method);
#endif
	
	png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
	if (f) {
	    rj.f = 0;
	    fclose(f);
	}

	{
	    int n = rowbytes * height, i, x, y, p, pln = rowbytes / width, pls = width * height;
	    double * data;
	    res = PROTECT(allocVector(REALSXP, rowbytes * height));
	    data = REAL(res);
	    for(y = 0; y < height; y++)
		for (x = 0; x < width; x++)
		    for (p = 0; p < pln; p++)
			data[y + x * height + p * pls] = ((double)row_pointers[y][x * pln + p]) / 255.0;
	    dim = allocVector(INTSXP, (pln > 1) ? 3 : 2);
	    INTEGER(dim)[0] = height;
	    INTEGER(dim)[1] = width;
	    if (pln > 1)
		INTEGER(dim)[2] = pln;
	    setAttrib(res, R_DimSymbol, dim);
	    UNPROTECT(1);
	}
    }
    
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

    return res;
}
