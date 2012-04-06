/*
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


 * @file testConstRangeAdapter.cpp
 * @author Tristan Roussillon (\c
 * tristan.roussillon@liris.cnrs.fr ) Laboratoire d'InfoRmatique en
 * Image et Systèmes d'information - LIRIS (CNRS, UMR 5205), CNRS,
 * France
 *
 *
 * @date 2012/02/06
 *
 * Functions for testing class ConstRangeAdapter
 *
 * This file is part of the DGtal library.
 */

#include <iostream>
#include <fstream>
#include <sstream>

#include "DGtal/base/Common.h"
#include "DGtal/base/BasicFunctors.h"
#include "DGtal/base/CConstSinglePassRange.h"
#include "DGtal/base/ConstRangeAdapter.h"

#include "DGtal/topology/KhalimskySpaceND.h"
#include "DGtal/topology/SCellsFunctors.h"
#include "DGtal/kernel/BasicPointFunctors.h"



using namespace std;
using namespace DGtal;


/*
 * Ranges
 *
 */
template <typename Range>
bool testRange(const Range &aRange)
{

  trace.info() << endl;
  trace.info() << "Testing Range" << endl;
  
  typedef typename IteratorCirculatorTraits<typename Range::ConstIterator>::Value Value; 
  std::vector<Value> v1,v2,v3,v4; 
  
  {
    trace.info() << "Forward" << endl;
    typename Range::ConstIterator i = aRange.begin();
    typename Range::ConstIterator end = aRange.end();
    for ( ; i != end; ++i) {
      cout << *i << " ";
      v1.push_back(*i); 
    }
    cout << endl; 
  }
  {
    trace.info() << "Backward" << endl;
    typename Range::ConstReverseIterator i = aRange.rbegin();
    typename Range::ConstReverseIterator end = aRange.rend();
    for ( ; i != end; ++i) {
      cout << *i << " ";
      v2.push_back(*i); 
    }
    cout << endl; 
  }
  {
    trace.info() << "Circulator" << endl;
    typename Range::ConstCirculator c = aRange.c();
    typename Range::ConstCirculator cend = aRange.c();
    if (isNotEmpty(c,cend)) 
      {
	do 
	  {
	    cout << *c << " ";
	    v3.push_back(*c);
	    c++;
	  } while (c!=cend); 
      }
    cout << endl; 
  }
  
  {
    trace.info() << "Reverse Circulator" << endl;
    typename Range::ConstReverseCirculator c = aRange.rc();
    typename Range::ConstReverseCirculator cend = aRange.rc();
    if (isNotEmpty(c,cend)) 
      {
	do 
	  {
	    cout << *c << " ";
	    v4.push_back(*c);
	    c++;
	  } while (c!=cend); 
      }
    cout << endl; 
  }

  return ( std::equal(v1.begin(),v1.end(),v3.begin())
	   && std::equal(v2.begin(),v2.end(),v4.begin())
	   && std::equal(v1.begin(),v1.end(),v2.rbegin())
	   && std::equal(v3.begin(),v3.end(),v4.rbegin()) );
}


template <typename Range>
void testRangeConceptChecking()
{
  BOOST_CONCEPT_ASSERT(( CConstSinglePassRange<Range> ));
}

/*
 * Standard services - public :
 */

int main( int argc, char** argv )
{
  trace.beginBlock ( "Testing class ConstRangeAdapter" );
  trace.info() << "Args:";
  for ( int i = 0; i < argc; ++i )
    trace.info() << " " << argv[ i ];
  trace.info() << endl;

  //1) simple range of integers
  const int n = 10; 
  std::vector<int> v;
  std::back_insert_iterator<std::vector<int> > ito(v); 
  for (int i = 1; i < n; ++i) 
      *ito++ = i;

  typedef ConstRangeAdapter<std::vector<int>::iterator, DefaultFunctor, int > SimpleRange; 
  DefaultFunctor df; 
  SimpleRange r1(v.begin(), v.end(), df); 

  //2) thresholded range of integers
  typedef ConstRangeAdapter<std::vector<int>::iterator, Thresholder<int>, bool > BoolRange; 
  Thresholder<int> t(n/2);  
  BoolRange r2(v.begin(), v.end(), t); 

  //3) range of signed cells...
  typedef KhalimskySpaceND<3> K; 
  typedef K::Point Point3; 
  vector<K::SCell> v3; 
  K ks; 
  v3.push_back(ks.sCell(Point3(1,1,0))); 
  v3.push_back(ks.sCell(Point3(2,1,1))); 
  v3.push_back(ks.sCell(Point3(3,1,2))); 
  //... transformed into inner voxels,
  //which are projected into 2d points
  typedef SpaceND<2> S;
  typedef S::Point Point2; 
  SCellToInnerPoint<K> f(ks); 
  Projector<S> p; 
  Composer<SCellToInnerPoint<K>,Projector<S>,Point2> c(f,p); 

  typedef ConstRangeAdapter<std::vector<K::SCell>::iterator, 
    Composer<SCellToInnerPoint<K>,Projector<S>,Point2>, Point2 > PointRange; 
  PointRange r3(v3.begin(), v3.end(), c); 
 
  /////////// concept checking
  testRangeConceptChecking<SimpleRange>();
  testRangeConceptChecking<BoolRange>();
  testRangeConceptChecking<PointRange>();

  /////////// iterators tests
  bool res = testRange(r1) 
    && testRange(r2) 
    && testRange(r3); 

  trace.emphase() << ( res ? "Passed." : "Error." ) << endl;
  trace.endBlock();  
  return res ? 0 : 1;
}