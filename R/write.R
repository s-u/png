writePNG <- function(image, target = raw()) {
  if (inherits(target, "connection")) {
    r <- .Call("write_png", image, raw(), PACKAGE="png")
    writeBin(r, target)
    invisible(NULL)
  } else invisible(.Call("write_png", image, if (is.raw(target)) target else path.expand(target), PACKAGE="png"))
}
