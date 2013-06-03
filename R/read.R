readPNG <- function(source, native=FALSE, info=FALSE)
  .Call(read_png, if (is.raw(source)) source else path.expand(source), native, info)
