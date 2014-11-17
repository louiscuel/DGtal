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

#pragma once

/**
 * @file GeneralizedVoronoiCovarianceMeasure.h
 * @author Jacques-Olivier Lachaud (\c jacques-olivier.lachaud@univ-savoie.fr )
 * Laboratory of Mathematics (CNRS, UMR 5127), University of Savoie, France
 *
 * @date 2014/02/09
 *
 * Header file for module GeneralizedVoronoiCovarianceMeasure.cpp
 *
 * This file is part of the DGtal library.
 */

#if defined(GeneralizedVoronoiCovarianceMeasure_RECURSES)
#error Recursive header files inclusion detected in GeneralizedVoronoiCovarianceMeasure.h
#else // defined(GeneralizedVoronoiCovarianceMeasure_RECURSES)
/** Prevents recursive inclusion of headers. */
#define GeneralizedVoronoiCovarianceMeasure_RECURSES

#if !defined GeneralizedVoronoiCovarianceMeasure_h
/** Prevents repeated inclusion of headers. */
#define GeneralizedVoronoiCovarianceMeasure_h

//////////////////////////////////////////////////////////////////////////////
// Inclusions
#include <cmath>
#include <iostream>
#include "DGtal/base/Common.h"
#include "DGtal/math/BasicMathFunctions.h"
#include "DGtal/kernel/BasicPointPredicates.h"
#include "DGtal/kernel/domains/HyperRectDomain.h"
#include "DGtal/kernel/SimpleMatrix.h"
#include "DGtal/images/ImageContainerBySTLVector.h"
#include "DGtal/geometry/volumes/distance/PowerMap.h"
#include "DGtal/geometry/volumes/distance/ExactPredicateLpPowerSeparableMetric.h"
#include "DGtal/geometry/volumes/CubicalSubdivision.h"
//////////////////////////////////////////////////////////////////////////////

namespace DGtal
{
  /**
     The hat function of value v0 at 0 with a linear decrease to 0 at value r.
     A function Scalar -> Scalar.
  */
  template <typename TScalar>
  struct HatFunction {
    typedef TScalar Scalar;
    typedef Scalar argument_type;
    typedef Scalar value_type;

    Scalar myV0;
    Scalar myV0OverR;
    Scalar myR;

  public:
    HatFunction( Scalar v0, Scalar r ) 
      : myV0( v0 ), myV0OverR( v0 / r ), myR( r ) {}
    Scalar operator()( Scalar d ) const
    { // d >= 0
      ASSERT( d >= 0.0 );
      if ( d >= myR ) return 0.0;
      return myV0 - myV0OverR * d;
    }
  };

  /**
     The ball constant function of value v0 between 0 and r, 0 elsewhere.
     A function Scalar -> Scalar.
  */
  template <typename TScalar>
  struct BallConstantFunction {
    typedef TScalar Scalar;
    typedef Scalar argument_type;
    typedef Scalar value_type;

    Scalar myV0;
    Scalar myR;

  public:
    BallConstantFunction( Scalar v0, Scalar r ) 
      : myV0( v0 ), myR( r ) {}
    Scalar operator()( Scalar d ) const
    {// d >= 0
      ASSERT( d >= 0.0 );
      return ( d > myR ) ? 0.0 : myV0;
    }
  };

  /**
     The hat function of value v0 at point 0 with a linear decrease to 0 at distance r.
     A function Point -> Scalar.
  */
  template <typename TPoint, typename TScalar>
  struct HatPointFunction {
    typedef TPoint Point;
    typedef TScalar Scalar;
    typedef Point argument_type;
    typedef Scalar value_type;

    Scalar myV0;
    Scalar myV0OverR;
    Scalar myR;
    Scalar myR2;

  public:
    HatPointFunction( Scalar v0, Scalar r ) 
      : myV0( v0 ), myV0OverR( v0 / r ), myR( r ), myR2( r*r ) {}
    Scalar operator()( const Point& p ) const
    {
      Scalar d = 0;
      for ( typename Point::ConstIterator it = p.begin(), itE = p.end(); it != itE; ++it )
        d += BasicMathFunctions::square( (Scalar) *it );
      if ( d >= myR2 ) return 0.0;
      d = sqrt( d );
      return myV0 - myV0OverR * d;
    }
  };

  /**
     The ball constant function of value v0 between norm 0 and r, 0 elsewhere.
     A function Point -> Scalar.
  */
  template <typename TPoint, typename TScalar>
  struct BallConstantPointFunction {
    typedef TPoint Point;
    typedef TScalar Scalar;
    typedef Point argument_type;
    typedef Scalar value_type;

    Scalar myV0;
    Scalar myR2;

  public:
    BallConstantPointFunction( Scalar v0, Scalar r ) 
      : myV0( v0 ), myR2( r*r ) {}
    Scalar operator()( const Point& p ) const
    {
     Scalar d = 0;
      for ( typename Point::ConstIterator it = p.begin(), itE = p.end(); it != itE; ++it )
        d += BasicMathFunctions::square( (Scalar) *it );
      return ( d > myR2 ) ? 0.0 : myV0;
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  // template class GeneralizedVoronoiCovarianceMeasure
  /**
   * Description of template class 'GeneralizedVoronoiCovarianceMeasure' <p>
   * \brief Aim: This class precomputes the Voronoi Covariance Measure
   * of a set of points. It can compute the covariance measure of an
   * arbitrary function with given support.
   *
   * You may obtain the whole sequence (Point,VCM) by accessing the
   * map \ref vcmMap.
   *
   * @note Documentation in \ref moduleVCM_sec2.
   *
   * @tparam TSpace type of Digital Space (model of CSpace).
   *
   * @tparam TSeparableMetric a model of CSeparableMetric used for
   * computing the Voronoi map (e.g. Euclidean metric is
   * DGtal::ExactPredicateLpSeparableMetric<TSpace, 2> )
   */
  template <typename TSpace>
  class GeneralizedVoronoiCovarianceMeasure
  {
    BOOST_CONCEPT_ASSERT(( CSpace< TSpace > ));

  public:
    typedef TSpace Space;                         ///< the type of digital space
    typedef ExactPredicateLpPowerSeparableMetric<Space,2> Metric; // L2-metric
    typedef typename Space::Point Point;          ///< the type of digital point
    typedef typename Space::Vector Vector;        ///< the type of digital vector
    typedef typename Space::Size Size;            ///< the type for counting elements
    typedef typename Space::Integer Integer;      ///< the type of each digital point coordinate, some integral type
    typedef DGtal::HyperRectDomain<Space> Domain; ///< the type of rectangular domain of the VCM.
    typedef DGtal::ImageContainerBySTLVector< Domain, double > WeightImage;///< the type of rectangular domain of the VCM.
    typedef DGtal::ImageContainerBySTLVector< Domain, Vector > ProjectionImage;///< the type of rectangular domain of the VCM.
    typedef DGtal::ImageContainerBySTLVector<Domain,bool> CharacteristicSet; ///< the type of a binary image that is the characteristic function of K.
    typedef DGtal::CubicalSubdivision<Space> ProximityStructure; ///< the structure used for proximity queries.

    /**
       A predicate that returns 'true' whenever the given binary image contains 'true'.
       Model of CPointPredicate.
       @note Internal use.
    */
    struct CharacteristicSetPredicate {
      typedef CharacteristicSetPredicate Self;
      typedef typename Space::Point Point;          ///< the type of digital point
      CharacteristicSetPredicate() : ptrSet( 0 ) {}
      CharacteristicSetPredicate( ConstAlias<CharacteristicSet> aSet) : ptrSet( &aSet ) {}
      CharacteristicSetPredicate( const Self& other ) : ptrSet( other.ptrSet ) {}
      Self& operator=( const Self& other ) { ptrSet = other.ptrSet; return *this; }
      bool operator()( const Point& p ) const
      { 
        ASSERT( ptrSet != 0 );
        return (*ptrSet)( p );
      }
    private:
      const CharacteristicSet* ptrSet;
    };
    typedef DGtal::NotPointPredicate<CharacteristicSetPredicate> NotPredicate; ///< the type of the point predicate used by the voronoi map.
    typedef DGtal::PowerMap< WeightImage, Metric, ProjectionImage > Power;
// typedef DGtal::VoronoiMap<Space, NotPredicate, Metric > Voronoi; ///< the type of the Voronoi map.

    typedef double Scalar;                                    ///< the type for "real" numbers.
    typedef DGtal::SimpleMatrix< Scalar,
                                 Space::dimension, 
                                 Space::dimension > MatrixNN; ///< the type for nxn matrix of real numbers.
    typedef typename MatrixNN::RowVector VectorN;             ///< the type for N-vector of real numbers
    typedef std::vector<Point> PointContainer;                ///< the list of points
    typedef std::map<Point,MatrixNN> Point2MatrixNN;          ///< Associates a matrix to points.

    // ----------------------- Standard services ------------------------------
  public:

    /**
     * Constructor.
     *
     * @param _R the offset radius for the set of points. Voronoi cells
     * are intersected with this offset. The unit corresponds to a step in the digital space.
     *
     * @param _r (an upper bound of) the radius of the support of
     * forthcoming kernel functions (\f$ \chi_r \f$). The unit
     * corresponds to a step in the digital space. This parameter is
     * used for preparing the data structure that answers to proximity
     * queries.
     *
     * @param aMetric an instance of the metric.
     * @param verbose if 'true' displays information on ongoing computation.
     */
    GeneralizedVoronoiCovarianceMeasure( double _R, double _r, int _k, bool verbose = false );

    /**
     * Destructor.
     */
    ~GeneralizedVoronoiCovarianceMeasure();

    /// @return the parameter R in the VCM, i.e. the offset radius for
    /// the compact set K.
    Scalar R() const;
    /// @return the parameter r in VCM(chi_r), i.e. an upper bound for
    /// the diameter of the support of kernel functions.
    Scalar r() const;

    int k() const;
 
    /**
       Cleans intermediate data structure likes the characteristic set and the voronoi map.
       @note Further calls to voronoiMap are no more valid.
    */
    void clean();

    /**
       Computes the Voronoi Covariance Measure for the set of points given by range [itb,ite)
       
       @tparam PointInputIterator an input iterator on digital points.
       @param itb the start of the range
       @param ite the end of the range.
       @pre \a itb != \a ite.
       @pre [itb,ite) is a valid range (\a ite can be reached from \a itb).
    */
    template <typename PointInputIterator>
    void init( PointInputIterator itb, PointInputIterator ite );

    /// @return the domain of computation
    const Domain& domain() const;

    /// @return the current Voronoi map 
    /// @pre init must have been called before.
    const Power& powerMap() const;

    /// @return the Voronoi Covariance Matrix of each Voronoi cell as
    /// a map Point -> Matrix
    /// @note empty if \ref init has not been called.
    const Point2MatrixNN& gvcmMap() const;

    /**
       Computes the Voronoi Covariance Measure of the function \a chi_r.
       
       @tparam Point2ScalarFunction the type of a functor
       Point->Scalar. For instance HatPointFunction and
       BallConstantPointFunction are models of this type.

       @param chi_r the kernel function whose support is included in
       the cube centered on the origin with edge size 2r (see \ref
       GeneralizedVoronoiCovarianceMeasure).

       @param p the point where the kernel function is moved. It must lie within domain.
    */
    template <typename Point2ScalarFunction>
    MatrixNN measure( Point2ScalarFunction chi_r, Point p ) const;

    // ----------------------- Interface --------------------------------------
  public:

    /**
     * Writes/Displays the object on an output stream.
     * @param out the output stream where the object is written.
     */
    void selfDisplay ( std::ostream & out ) const;

    /**
     * Checks the validity/consistency of the object.
     * @return 'true' if the object is valid, 'false' otherwise.
     */
    bool isValid() const;

    // ------------------------- Protected Datas ------------------------------
  private:
    // ------------------------- Private Datas --------------------------------
  private:

    /// The parameter R in the VCM, i.e. the offset radius for the compact set K.
    double myBigR;  
    /// The parameter r in VCM(chi_r), i.e. an upper bound for the
    /// diameter of the support of kernel functions.
    double mySmallR;
    /// The parameter k for the k-distance
    int myk;    
    /// The metric chosen for the Voronoi map.
    Metric myMetric;
    /// Tells if it is verbose mode.
    bool myVerbose;
    /// The domain in which all computations are done.
    Domain myDomain;
    /// A binary image that defines the characteristic set of K.
    CharacteristicSet* myCharSet;
    /// Stores the voronoi map.
    Power* myPower;
    /// The map point -> VCM
    Point2MatrixNN myGVCM;
    /// The structure used for proximity queries.
    ProximityStructure* myProximityStructure;

    WeightImage myWeightImage;

    // ------------------------- Hidden services ------------------------------
  protected:

    /**
     * Constructor.
     * Forbidden by default (protected to avoid g++ warnings).
     */
    GeneralizedVoronoiCovarianceMeasure();

  private:

    /**
     * Copy constructor.
     * @param other the object to clone.
     * Forbidden by default.
     */
    GeneralizedVoronoiCovarianceMeasure ( const GeneralizedVoronoiCovarianceMeasure & other );

    /**
     * Assignment.
     * @param other the object to copy.
     * @return a reference on 'this'.
     * Forbidden by default.
     */
    GeneralizedVoronoiCovarianceMeasure & operator= ( const GeneralizedVoronoiCovarianceMeasure & other );

    // ------------------------- Internals ------------------------------------
  private:

  }; // end of class GeneralizedVoronoiCovarianceMeasure


  /**
   * Overloads 'operator<<' for displaying objects of class 'GeneralizedVoronoiCovarianceMeasure'.
   * @param out the output stream where the object is written.
   * @param object the object of class 'GeneralizedVoronoiCovarianceMeasure' to write.
   * @return the output stream after the writing.
   */
  template <typename TSpace>
  std::ostream&
  operator<< ( std::ostream & out, 
               const GeneralizedVoronoiCovarianceMeasure<TSpace> & object );

} // namespace DGtal


///////////////////////////////////////////////////////////////////////////////
// Includes inline functions.
#include "DGtal/geometry/volumes/estimation/GeneralizedVoronoiCovarianceMeasure.ih"

//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#endif // !defined GeneralizedVoronoiCovarianceMeasure_h

#undef GeneralizedVoronoiCovarianceMeasure_RECURSES
#endif // else defined(GeneralizedVoronoiCovarianceMeasure_RECURSES)
