\name{readPNG}
\alias{readPNG}
\title{
Read a PNG image file.
}
\description{
Reads an image from a PNG into a raster array.
}
\usage{
readPNG(fn)
}
\arguments{
  \item{fn}{Name of the file to read from.}
}
%\details{
%}
\value{
Array of the dimensions height x width x channels. If there is only
one channel the result is a matrix. The values are real between 0 and 1.
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
img=readPNG(system.file("img","Rlogo.png",package="png"))
plot(1:2,type='n',)
raster(img,1.2,1.27,1.8,1.73)
}
\keyword{IO}
