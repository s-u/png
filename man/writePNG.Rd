\name{writePNG}
\alias{writePNG}
\title{
Write a bitmap image in PNG format
}
\description{
Create a PNG image from an array or matrix.
}
\usage{
writePNG(image, target = raw())
}
\arguments{
  \item{image}{image represented by a real matrix or array with values
    in the range of 0 to 1. Values outside this range will be
    clipped. The object must be either two-dimensional (grayscale
    matrix) or three dimensional array (third dimension specifying the
    plane) and must have either one (grayscale), two (grayscale +
    alpha), three (RGB) or four (RGB + alpha) planes. (For alternative
    image specifications see deatils)}
  \item{target}{Either name of the file to write, a binary connection or
    a raw vector (\code{raw()} - the default - is good enough)
    indicating that the output should be a raw vector.}
}
\value{
Either NULL if the target is a file or a raw vector containing the
compressed PNG image if the target was a raw vector.
}
\details{
  \code{writePNG} takes an image as input and compresses it into PNG
  format. The image input is usually a matrix (for grayscale images -
  dimensions are width, height) or an array (for color and alpha
  images - dimensions are width, height, planes) of reals. The planes
  are interpreted in the sequence red, green, blue, alpha.

  Alternative representation of an image is of \code{nativeRaster} class
  which is an integer matrix with each entry representing one pixel in
  binary encoded RGBA format (as used internally by R). It can be
  obtained from \code{\link{readPNG}} using \code{native = TRUE}.

  Finally, \code{writePNG} also supports raw array containing the RGBA
  image as bytes. The dimensions of the raw array have to be planes,
  width, height (because the storage is interleaved). Currently only 4
  planes (RGBA) are supported and the processing is equivalent to that
  of a native raster.

  The result is either stored in a file (if \code{target} is a file
  name), in a raw vector (if \code{target} is a raw vector) or sent to a
  binary connection.
}
%\references{
%}
%\author{
%}
\note{
  Currently \code{writePNG} only produces 8-bit, deflate-compressed,
  non-quantized, non-interlaced images. Note in particular that
  \code{\link{readPNG}} can read 16-bit channels but storing them
  back using \code{writePNG} will strip the 8 LSB (irrelevant for
  display purposes but possibly relevant for use of PNG in
  signal-processing if the input is truly 16-bit wide).
}
\seealso{
\code{\link{readPNG}}
}
\examples{
# read a sample file (R logo)
img <- readPNG(system.file("img","Rlogo.png",package="png"))
# write the image into a raw vector
r <- writePNG(img)
# read it back again
img2 <- readPNG(r)
# it better be the same
identical(img, img2)
# try to write a native raster
img3 <- readPNG(system.file("img","Rlogo.png",package="png"), TRUE)
r2 <- writePNG(img3)
img4 <- readPNG(r2, TRUE)
identical(img3, img4)
}
\keyword{IO}
