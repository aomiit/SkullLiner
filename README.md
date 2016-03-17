# SkullLiner

![dental](http://starcolon.com/IMG/ARTWORK/dental-cinema.png)

A highly performant curve estimator for dental 
CT scan imagery diagnosis.

http://starcolon.com/dental-nerve-inspector/index.html

---

## Requirements

- [x] [OpenCV 2.x](http://opencv.org/)
- [x] [VTK](http://www.vtk.org/)
- [x] [JNI](https://en.wikipedia.org/wiki/Java_Native_Interface)


---

## Building the projects

Projects were created on Visual Studio 2005 back 
in around circa 2010. Building the projects on 
Windows should be fine.


---

## How algorithms work

Long story short:

- It reads in the input VTK image (Raw CT scan).
- It then boosts the image with log transformation.
- It then binarises the image, extracts the curve-like blobs.
- It fits the curve-like blobs with linear programming.
- It now has an estimated parametric curve of the image.


---

## Licence

MIT Licence
