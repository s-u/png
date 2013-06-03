writePNG <- function(image, target = raw(), dpi = NULL, asp = NULL) {
  if (inherits(target, "connection")) {
    r <- .Call(write_png, image, raw(), dpi, asp)
    writeBin(r, target)
    invisible(NULL)
  } else invisible(.Call(write_png, image, if (is.raw(target)) target else path.expand(target), dpi, asp))
}
