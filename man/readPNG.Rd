\name{readPNG}
\alias{readPNG}
\title{
Read a bitmap image stored in the PNG format
}
\description{
Reads an image from a PNG file/content into a raster array.
}
\usage{
readPNG(source)
}
\arguments{
  \item{source}{Either name of the file to read from or a raw vector
  representing the PNG file content.}
}
%\details{
%}
\value{
Array of the dimensions height x width x channels. If there is only
one channel the result is a matrix. The values are reals
between 0 and 1.

Most common files decompress into RGB (3 channels) or RGBA (4 channels).
}
%\references{
%}
%\author{
%}
%\note{
%}

\seealso{
\code{link{raster}}
}
\examples{
# read a sample file (R logo)
img <- readPNG(system.file("img","Rlogo.png",package="png"))
# if your R supports it, we'll plot it
if (exists("raster")) { # can plot only in R 2.11.0 and higher
  plot(1:2,type='n',)
  raster(img,1.2,1.27,1.8,1.73)
}
}
\keyword{IO}
