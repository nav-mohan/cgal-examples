#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Poisson_reconstruction_function.h>

#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>
#include <CGAL/Poisson_mesh_domain_3.h>
#include <CGAL/make_mesh_3.h>
#include <CGAL/facets_in_complex_3_to_triangle_mesh.h>
#include <CGAL/mst_orient_normals.h>
#include <CGAL/jet_estimate_normals.h>

#include <CGAL/Point_set_3.h>
// #include <CGAL/Point_set_processing_3.h>
#include <CGAL/edge_aware_upsample_point_set.h>

#include <CGAL/property_map.h>
#include <CGAL/IO/read_points.h>
#include <CGAL/compute_average_spacing.h>

#include <CGAL/Polygon_mesh_processing/distance.h>

#include <boost/iterator/transform_iterator.hpp>

#include <vector>
#include <fstream>
// Types
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::FT FT;
typedef Kernel::Point_3 Point;
typedef Kernel::Vector_3 Vector;
typedef std::pair<Point, Vector> Point_with_normal;
typedef CGAL::First_of_pair_property_map<Point_with_normal> Point_map;
typedef CGAL::Second_of_pair_property_map<Point_with_normal> Normal_map;
typedef Kernel::Sphere_3 Sphere;
typedef std::vector<Point_with_normal> PointList;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
typedef CGAL::Poisson_reconstruction_function<Kernel> Poisson_reconstruction_function;
typedef CGAL::Poisson_mesh_domain_3<Kernel> Mesh_domain;
typedef CGAL::Mesh_triangulation_3<Mesh_domain>::type Tr;
typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr> C3t3;
typedef CGAL::Mesh_criteria_3<Tr> Mesh_criteria;

typedef CGAL::Point_set_3<Point> Point_set;
typedef std::pair<Point, Vector> PointVectorPair;

int main(int argc, const char * argv[])
{
    float min_angle = 20.0, max_size = 0.5, rel_err = 0.1;
    if(argc == 4)
    {
      min_angle = atof(argv[1]);
      max_size = atof(argv[2]); 
      rel_err = atof(argv[3]); 
    }

    FT sm_angle = min_angle; // Min triangle angle in degrees.
    FT sm_radius = max_size; // Max triangle size w.r.t. point set average spacing.
    FT sm_distance = rel_err; // Surface Approximation error w.r.t. point set average spacing.

    // Reads the point set file in points[].
    // Note: read_points() requires an iterator over points
    // + property maps to access each point's position and normal.
    PointList points;
    std::string fname = "../bunny_with_normals.xyz";
    // std::string fname = "points_3/kitten.xyz";
    if(!CGAL::IO::read_points(CGAL::data_file_path(fname), std::back_inserter(points),
                          CGAL::parameters::point_map(Point_map())
                                           .normal_map (Normal_map())))
    {
      std::cerr << "Error: cannot read file input file!" << std::endl;
      return EXIT_FAILURE;
    }

    // perform upsampling 
    std::vector<Point_with_normal> upsampled;
    const double sharpness_angle = 5;   // control sharpness of the result.
    const double edge_sensitivity = 0.1;    // higher values will sample more points near the edges
    const double neighbor_radius = 0.2;  // initial size of neighborhood.
    const std::size_t number_of_output_points = points.size() * 10;

    printf("START UPSAMPLING\n");
    CGAL::edge_aware_upsample_point_set<CGAL::Parallel_if_available_tag>(
        points,
        std::back_inserter(upsampled),
        CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointVectorPair>()).
        normal_map(CGAL::Second_of_pair_property_map<PointVectorPair>()).
        sharpness_angle(sharpness_angle).
        edge_sensitivity(edge_sensitivity).
        neighbor_radius(neighbor_radius).
        number_of_output_points(number_of_output_points));
    
        printf("Upsampled %ld --> %ld\n",points.size(), upsampled.size());

        points = std::move(upsampled);

for (auto& pn : points) 
{
    const Point& p = pn.first;
    const Vector& n = pn.second;
    if (n.squared_length() < 1e-12) 
      std::cerr << "Zero-length normal detected!\n";
}

// Remove NaNs
points.erase(
  std::remove_if(
    points.begin(), points.end(),
    [](const Point_with_normal& pn) {
            auto p = pn.first;
            auto n = pn.second;
            return (
              std::isnan(p.x()) || std::isnan(p.y()) || std::isnan(p.z()) || 
              std::isnan(n.x()) || std::isnan(n.y()) || std::isnan(n.z()) 
            );
        }),
    points.end()
);


// Re-estimate + re-orient normals
CGAL::jet_estimate_normals<CGAL::Sequential_tag>(
    points, 24,
    CGAL::parameters::point_map(Point_map()).normal_map(Normal_map())
);

auto unoriented =
    CGAL::mst_orient_normals(points, 24,
        CGAL::parameters::point_map(Point_map()).normal_map(Normal_map()));

points.erase(unoriented, points.end());

printf("Removed invalid points to %ld\n",points.size());

    // Creates implicit function from the read points using the default solver.

    // Note: this method requires an iterator over points
    // + property maps to access each point's position and normal.
    printf("START POISSON\n");
    Poisson_reconstruction_function function(points.begin(), points.end(), Point_map(), Normal_map());

    // Computes the Poisson indicator function f()
    // at each vertex of the triangulation.
    if ( ! function.compute_implicit_function() ) return EXIT_FAILURE;
    printf("DONE POISSON\n");

    // Computes average spacing
    FT average_spacing = CGAL::compute_average_spacing<CGAL::Sequential_tag>(points, 6 /* knn = 1 ring */,CGAL::parameters::point_map (Point_map()));
    printf("average spacing = %f\n",average_spacing);

    //Computes implicit function bounding sphere radius.
    Sphere bsphere = function.bounding_sphere();
    FT radius = std::sqrt(bsphere.squared_radius());
    printf("bsphere-radius = %f\n",radius);

    FT sm_sphere_radius = 2.0 * radius;
    FT sm_dichotomy_error = sm_distance*average_spacing/1000.0; // Dichotomy error must be << sm_distance
    printf("dicho-error = %f\n",sm_dichotomy_error);

    // Defines surface mesh generation criteria
    Mesh_criteria criteria(CGAL::parameters::facet_angle = sm_angle,
                           CGAL::parameters::facet_size = sm_radius*average_spacing,
                           CGAL::parameters::facet_distance = sm_distance*average_spacing);

    // Defines mesh domain
    Mesh_domain domain = Mesh_domain::create_Poisson_mesh_domain(function, bsphere,
        CGAL::parameters::relative_error_bound(sm_dichotomy_error / sm_sphere_radius));

    // Generates mesh with manifold option
    printf("START MESHING\n");
    C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria,
                                        CGAL::parameters::surface_only()
                                        .manifold_with_boundary());

    printf("DONE MESHING\n");
    const Tr& tr = c3t3.triangulation();
    if(tr.number_of_vertices() == 0) return EXIT_FAILURE;

    /// [PMP_distance_snippet]
    // computes the approximation error of the reconstruction
    Polyhedron output_mesh;
    CGAL::facets_in_complex_3_to_triangle_mesh(c3t3, output_mesh);
    double max_dist =
      CGAL::Polygon_mesh_processing::approximate_max_distance_to_point_set
      (output_mesh,
       CGAL::make_range (boost::make_transform_iterator
                         (points.begin(), CGAL::Property_map_to_unary_function<Point_map>()),
                         boost::make_transform_iterator
                         (points.end(), CGAL::Property_map_to_unary_function<Point_map>())),
       3000);
    std::cout << "Max distance to point_set: " << max_dist << std::endl;
    /// [PMP_distance_snippet]


    // saves reconstructed surface mesh
    std::ofstream out("bunny.off");
    out << output_mesh;

    return EXIT_SUCCESS;
}
