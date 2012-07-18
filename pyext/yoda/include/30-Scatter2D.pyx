cdef extern from "YODA/Scatter2D.h" namespace "YODA":
    cdef cppclass cScatter2D "YODA::Scatter2D" (cAnalysisObject):
        cScatter2D()
        cScatter2D(vector[cPoint2D]&, string, string)
        cScatter2D(cScatter2D &s)

        size_t numPoints()
        vector[cPoint2D] points()
        cPoint2D& point(size_t i)



cdef class Scatter2D(AnalysisObject):

    def __init__(self, *args, **kwargs):
        """
        Scatter2D constructor. Several sets of arguments are permitted:

        * Scatter2D() -- default constructor. Not usually useful in Python, due to availability of None.
        * Scatter2D(points[, path, title]) -- explicit construction from a list of points.
        * Scatter2D(xs, ys[, path, title]) -- constructing points from lists of x and y positions (no errs).
        * Scatter2D(xs, ys, exs, eys[, path, title]) -- constructing points from lists of x and y positions and errs (errs can be pairs).
        * Scatter2D(xs, ys, ex-s, ex+s, ey-s, ey+s[, path, title]) -- constructing points from lists of x and y positions and errs.

        The path and title arguments are optional, and may either be specified via the
        positional parameters or via explicit keyword arguments, e.g. path='/foo/bar'.
        """
        #self._dealloc = True
        cdef:
            # size_t N = len(points)
            # int i
            Point2D item
            vector[cPoint2D] point_vector
            cScatter2D* scatter
            char* path = '/'
            char* title = ''

        ## Permit path and title specification via kwargs
        if "path" in kwargs:
            path = kwargs["path"]
        if "title" in kwargs:
            path = kwargs["title"]

        ## Trigger different construction methods depending on Python args
        # TODO: Map copy constructors, esp. the path-resetting one
        if len(args) == 0:
            self.setptr(new cScatter2D())
        elif len(args) == 1:
            for point in args[0]:
                item = point
                point_vector.push_back( item.ptr()[0] )
        elif len(args) == 2:
            # Scatter2D(xs, ys[, path, title])
            assert len(args[0]) == len(args[1])
            for i in xrange(args[0]):
                item = Point2D(args[0][i], args[1][i])
                point_vector.push_back( item.ptr()[0] )
        elif len(args) == 4:
            # Scatter2D(xs, ys, exs, eys[, path, title])
            assert len(args[0]) == len(args[1]) == len(args[2]) == len(args[3])
            for i in xrange(args[0]):
                item = Point2D(args[0][i], args[1][i], args[2][i], args[3][i])
                point_vector.push_back( item.ptr()[0] )
        elif len(args) == 6:
            # Scatter2D(xs, ys, ex-s, ex+s, ey-s, ey+s[, path, title])
            assert len(args[0]) == len(args[1]) == len(args[2]) == len(args[3])
            for i in xrange(args[0]):
                item = Point2D(args[0][i], args[1][i], args[2][i], args[3][i], args[4][i], args[5][i])
                point_vector.push_back( item.ptr()[0] )
        else:
            raise ValueError('Wrong number of values: can take 2, 4, or 6 parameters')

        scatter = new cScatter2D(point_vector, string(path), string(title))
        self.setptr(scatter, True)


    @property
    def numPoints(self):
        return self.ptr().numPoints()

    cdef cScatter2D * ptr(self):
        return <cScatter2D *> self.thisptr

    def __getitem__(self, args):
        return self.points.__getitem__(args)

    def copy(self):
        return Scatter2D_fromptr(new cScatter2D(self.ptr()[0]), True)

    @property
    def points(self):
        cdef size_t i
        cdef Point2D pt
        if self._points:
            return self._points
        else:
            out = []

            for i in range(self.ptr().numPoints()):
                pt = Point2D_fromptr(& self.ptr().point(i))
                out.append(pt)

            out = tuple(out)
            self._points = out
            return out

    def __repr__(self):
        return '<Scatter2D>'


# dealloc decides whether or not the python object is responsible for freeing
# used memory. Most times, it's not - we're just wrapping a C++ instance.
# However, the same classes should be used for both wrapping and creating.

# It is important that we do free allocated C++ memory, because otherwise we can
# quickly end up with memory leaks.

cdef Scatter2D Scatter2D_fromptr(cScatter2D* ptr, dealloc = False):
    cdef Scatter2D scatter = Scatter2D.__new__(Scatter2D)
    return scatter.setptr(ptr, False)
