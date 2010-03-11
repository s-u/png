readPNG <- function(fn)	
  .Call("read_png", fn, PACKAGE="png")
