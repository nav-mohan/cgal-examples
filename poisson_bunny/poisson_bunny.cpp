// poisson surface reconstruction requires oriented normals
// bunny.csv does not provide normals
// we must estimate normals first

#include <CGAL/Simple_cartesian.h>
#include <CGAL/poisson_surface_reconstruction.h>
#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <fstream>
#include <vector>

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point;
typedef Kernel::Vector_3 Vector;
typedef CGAL::Poisson_reconstruction_function<Kernel> Poisson_reconstruction_function;

// Types for output mesh
typedef CGAL::Surface_mesh_default_triangulation_3 STr;
typedef CGAL::Surface_mesh_complex_2_in_triangulation_3<STr> C2t3;
typedef CGAL::Implicit_surface_3<Kernel, Poisson_reconstruction_function> Surface_3;

int main() {
    std::vector<Point> points;
    std::vector<Vector> normals;

    std::ifstream in("../bunny_with_normals.xyz");
    double x, y, z, nx, ny, nz;
    while (in >> x >> y >> z >> nx >> ny >> nz) {
        points.emplace_back(x, y, z);
        normals.emplace_back(nx, ny, nz);
    }

    // Fit the implicit function
    Poisson_reconstruction_function function(points.begin(), points.end(), normals.begin());

    // Compute bounding sphere
    Kernel::Sphere_3 bsphere = function.bounding_sphere();
    double radius = std::sqrt(bsphere.squared_radius());

    // Define the implicit surface = zero level-set of function
    // Surface_3 surface(function, CGAL::Sphere_3(bsphere.center(), radius * radius), 1e-6);
    Surface_3 surface(function,bsphere,0.01);

    // Generate mesh
    STr tr;
    C2t3 c2t3(tr);
    CGAL::Surface_mesh_default_criteria_3<STr> criteria(30.,  // Angle (min degrees)
                                                        0.5,  // Radius
                                                        0.5); // Distance

    // CGAL::make_surface_mesh(c2t3, surface, criteria, CGAL::Manifold_with_boundary_tag());
    CGAL::make_surface_mesh(c2t3,surface,criteria,CGAL::Manifold_tag());

    std::cout << "Final number of facets: " << tr.number_of_cells() << std::endl;

    std::ofstream out("poisson_bunny.off");
    // CGAL::output_surface_facets_to_off(out, c2t3);
}

