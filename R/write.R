writePNG <- function(image, target)
  invisible(.Call("write_png", image, if (is.raw(target)) target else path.expand(target), PACKAGE="png"))
