#include <boost/multiprecision/gmp.hpp>  // GMP backend for Boost Multiprecision
#include <CGAL/CORE/BigInt.h>            // CGAL's BigInt if it's used

#include <CGAL/Simple_cartesian.h>
#include <CGAL/point_generators_2.h>

#include <iostream>

#include <CGAL/Polygon_2.h>


typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;

int main() 
{
    Point_2 p(1, 2), q(3, 4);
    std::cout << "p = " << p << ", q = " << q << std::endl;
    std::cout << "Distance squared = " << CGAL::squared_distance(p, q) << std::endl;

    return 0;
}
