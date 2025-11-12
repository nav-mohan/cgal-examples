#include <boost/multiprecision/gmp.hpp>
#include <CGAL/CORE/BigInt.h>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polychain_2.h>

#include <iostream>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Polygon_2<K> Polygon_2;

int main()
{
    K::Point_2 a(0,0);
    K::Point_2 b(1,0);
    K::Point_2 c(0,1);
    CGAL::Orientation o = CGAL::orientation(a,c,b);

    if (o == CGAL::LEFT_TURN) std::cout << "LEFT TURN" << std::endl;
    if (o == CGAL::RIGHT_TURN) std::cout << "RIGHT TURN" << std::endl;
    if (o == CGAL::COLLINEAR) std::cout << "COLLINEAR" << std::endl;

    Polygon_2 poly;
    poly.push_back(K::Point_2(0,0));
    poly.push_back(K::Point_2(1,0));
    poly.push_back(K::Point_2(1,1));
    poly.push_back(K::Point_2(0,1));

    std::cout << "IS CONVEX: " << poly.is_convex() << std::endl;
    std::cout << "AREA " << poly.area() << std::endl;


}