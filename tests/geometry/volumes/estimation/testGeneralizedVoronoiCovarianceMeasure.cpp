/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

/**
 * @file testVoronoiCovarianceMeasure.cpp
 * @ingroup Tests
 * @author Jacques-Olivier Lachaud (\c jacques-olivier.lachaud@univ-savoie.fr )
 * Laboratory of Mathematics (CNRS, UMR 5127), University of Savoie, France
 *
 * @date 2014/02/09
 *
 * Functions for testing class VoronoiCovarianceMeasure.
 *
 * This file is part of the DGtal library.
 */

///////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <vector>
#include "DGtal/base/Common.h"
#include "DGtal/helpers/StdDefs.h"
#include "DGtal/geometry/volumes/estimation/GeneralizedVoronoiCovarianceMeasure.h"
#include "DGtal/geometry/volumes/CubicalSubdivision.h"

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Functions for testing class VoronoiCovarianceMeasure
///////////////////////////////////////////////////////////////////////////////
using namespace DGtal;
/**
 * Example of a test. To be completed.
 *
 */
bool testGeneralizedVoronoiCovarianceMeasure()
{
  unsigned int nbok = 0;
  unsigned int nb = 0;
  
  using namespace DGtal;
  using namespace DGtal::Z3i; // gets Space, Point, Domain
  //trace.beginBlock ( "testGeneralizedVoronoiCovarianceMeasure" );
  typedef ExactPredicateLpPowerSeparableMetric<Space,2> Metric;
  typedef GeneralizedVoronoiCovarianceMeasure<Space> VCM;
  typedef VCM::MatrixNN Matrix;
  Point a(0,0);
  Point c(32,32,32);
  Domain domain(a,c);
    typedef typename Space::Point Point;          ///< the type of digital point
    typedef typename Space::Size Size;            ///< the type for counting elements
    typedef typename Space::Integer Integer;      ///< the type of each digital point coordinate, some integral type
    typedef DGtal::HyperRectDomain<Space> Domain; ///< the type of rectangular domain of the VCM.
  typedef VCM::WeightImage WeightImage;
  typedef VCM::WeightImage WeightImage2;
//WeightImage pts(Domain.begin(),Domain.end());
  std::vector<VCM::Point> pts;
  pts.push_back( Point( 10,10,10 ) );
  pts.push_back( Point( 10,10,11 ) );
  pts.push_back( Point( 10,11,11 ) );
  pts.push_back( Point( 11,11,12 ) );
  pts.push_back( Point( 11,12,12 ) );
  pts.push_back( Point( 11,12,12 ) );
  pts.push_back( Point( 12,12,12 ) );
  pts.push_back( Point( 20,20,15 ) );
  pts.push_back( Point( 30,10,25 ) );
  pts.push_back( Point( 25,25,20 ) );



  Metric l2;
  VCM vcm( 5.0, 4.0, 8 , true );
//std::cerr << "aaaaaaaaaaaaa"<<std::endl; 
   vcm.init( pts.begin(), pts.end() );
//std::cerr << "bbbbbbbbbbbbbb"<<std::endl; 
  Domain d = vcm.domain();
  double sum_dist = 0.0;
  std::map<Point,int> sizeCells;
  for ( Domain::ConstIterator it = d.begin(), itE = d.end(); it != itE; ++it )
    {
   //   std::cerr << l2.powerDistance( *it, vcm.powerMap()( *it ), 0.1 ) <<std::endl;
      sum_dist += l2.powerDistance( *it, vcm.powerMap()( *it ), 0.1 );
      sizeCells[ vcm.powerMap()( *it ) ] += 1;
    }
  double avg_dist = ( sum_dist / d.size() );
  trace.info() << "Average distance is " << avg_dist << std::endl;
  nbok += ( 9 <= avg_dist && avg_dist <= 11 ) ? 1 : 0; 
  nb++;
  trace.info() << "(" << nbok << "/" << nb << ") "
               << "9 <= " << avg_dist << " <= 11 " << std::endl;
  for ( std::map<Point,int>::const_iterator it = sizeCells.begin(), itE = sizeCells.end();
        it != itE; ++it )
    trace.info() << "Nb(" << it->first << ")=" << it->second << std::endl;
  nbok += sizeCells.size() == 9 ? 1 : 0;
  nb++;
  trace.info() << "(" << nbok << "/" << nb << ") "
               << "sizeCells.size() == 10" << std::endl;


  HatPointFunction< Point, double > chi_r( 1.0, 4.0 );
  Matrix vcm_r = vcm.measure( chi_r, Point( 10,10,10 ) );
  trace.info() << "- vcm_r.row(0) = " << vcm_r.row( 0 ) << std::endl;
  trace.info() << "- vcm_r.row(1) = " << vcm_r.row( 1 ) << std::endl;
  trace.info() << "- vcm_r.row(2) = " << vcm_r.row( 2 ) << std::endl;
  trace.endBlock();
  return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Standard services - public :

int main( int /* argc */, char** /* argv */ )
{
  using namespace std;
  using namespace DGtal;
 // trace.beginBlock ( "Testing GeneralizedoronoiCovarianceMeasure ..." );
  bool res = testGeneralizedVoronoiCovarianceMeasure();
  //trace.emphase() << ( res ? "Passed." : "Error." ) << endl;
  //trace.endBlock();
return 1; 

// return res ? 0 : 1;
}
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
