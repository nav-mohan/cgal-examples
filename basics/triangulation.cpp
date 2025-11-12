#include <boost/multiprecision/gmp.hpp>
#include <CGAL/CORE/BigInt.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <iostream>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Delaunay_triangulation_2<K> Delaunay;
typedef K::Point_2 Point;

int main()
{
    std::vector<Point> pts = {{0,0}, {1,0}, {0,1}, {1,1}};
    Delaunay dt(pts.begin(), pts.end());
    printf("------TRIANGLES-------\n");
    for ( auto f = dt.finite_faces_begin(); f != dt.finite_faces_end(); ++f)
    {
        std::cout << dt.triangle(f) << std::endl;
    }
    printf("------POINTS-------\n");

    for (auto v = dt.finite_vertices_begin(); v != dt.finite_vertices_end(); ++v)
        std::cout << v->point() << std::endl;


    printf("------SEGMENTS-------\n");
    for (auto e = dt.finite_edges_begin(); e != dt.finite_edges_end(); ++e) 
        std::cout << dt.segment(*e) << std::endl;
 
    return 0;
}
