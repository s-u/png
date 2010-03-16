\name{writePNG}
\alias{writePNG}
\title{
Write a bitmap image in PNG format
}
\description{
Create a PNG image from an array or matrix.
}
\usage{
writePNG(image, target)
}
\arguments{
  \item{image}{image represented by a real matrix or array with values
  in the range of 0 to 1. Values outside this range will be
  clipped. The object must be either two-dimensional (grayscale
  matrix) or three dimensional array (third dimension specifying the
  plane) and must have either one (grayscale), two (grayscale +
  alpha), three (RGB) or four (RGB + alpha) planes.}
  \item{target}{Either name of the file to write to or a raw vector
  (\code{raw()} is good enough) indicating that the output should be a
  raw vector.}
}
%\details{
%}
\value{
Either NULL if the target is a file or a raw vector containing the
compressed PNG image if the target was a raw vector.
}
%\references{
%}
%\author{
%}
%\note{
%}

\seealso{
\code{link{readPNG}}
}
\examples{
# read a sample file (R logo)
img <- readPNG(system.file("img","Rlogo.png",package="png"))
# write the image into a raw vector
r <- writePNG(img, raw())
# read it back again
img2 <- readPNG(r)
# it better be the same
identical(img, img2)
}
\keyword{IO}
