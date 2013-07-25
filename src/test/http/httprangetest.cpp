/*****************************************************************************
*    Open LiteSpeed is an open source HTTP server.                           *
*    Copyright (C) 2013  LiteSpeed Technologies, Inc.                        *
*                                                                            *
*    This program is free software: you can redistribute it and/or modify    *
*    it under the terms of the GNU General Public License as published by    *
*    the Free Software Foundation, either version 3 of the License, or       *
*    (at your option) any later version.                                     *
*                                                                            *
*    This program is distributed in the hope that it will be useful,         *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
*    GNU General Public License for more details.                            *
*                                                                            *
*    You should have received a copy of the GNU General Public License       *
*    along with this program. If not, see http://www.gnu.org/licenses/.      *
*****************************************************************************/
#ifdef RUN_TEST
 
#include <http/httprange.h>
#include <http/httpstatuscode.h>
#include "test/unittest-cpp/UnitTest++/src/UnitTest++.h"
SUITE(YourSuitqqqqeName)
{

TEST(Test_httprangetest)
{
    int el = 2000;
    HttpRange range( el );

    const char * psRange = "bytes= 0 -\t0\t ,1234-23456,-10,10-,1999-1999";
    CHECK( range.parse( psRange ) == 0 );
    CHECK( range.count() == 5 );

    long b, e;
    CHECK( range.getContentOffset( 0, b, e ) == 0 );
    CHECK( b == 0 );
    CHECK( e == 1 );
    CHECK( range.getContentOffset( 1, b, e ) == 0 );
    CHECK( b == 1234 );
    CHECK( e == el );
    CHECK( range.getContentOffset( 2, b, e ) == 0 );
    CHECK( b == el-10 );
    CHECK( e == el );
    CHECK( range.getContentOffset( 3, b, e ) == 0 );
    CHECK( b == 10 );
    CHECK( e == el );
    CHECK( range.getContentOffset( 4, b, e ) == 0 );
    CHECK( b == 1999 );
    CHECK( e == el );
    
    char crs[1000];
    int len;
    range.beginMultipart();
    CHECK( len = range.getPartHeader( 0, "text/html", crs, 1000 ));
    int partLen = range.getPartLen( 0, 9 );
    CHECK( partLen == len + 1 );
    CHECK( len = range.getPartHeader( 1, "text/html", crs, 1000 ) );
    partLen = range.getPartLen( 1, 9 );
    CHECK( partLen == len + 2000 - 1234 );    

    CHECK( range.getContentRangeString( 0, crs, 99 ) > 0);
    CHECK( strcmp( crs, "bytes 0-0/2000" ) == 0 );
    CHECK( range.getContentRangeString( 1, crs, 99 ) > 0);
    CHECK( strcmp( crs, "bytes 1234-1999/2000" ) == 0 );
    CHECK( range.getContentRangeString( 2, crs, 99 ) > 0);
    CHECK( strcmp( crs, "bytes 1990-1999/2000" ) == 0 );
    CHECK( range.getContentRangeString( 3, crs, 99 ) > 0);
    CHECK( strcmp( crs, "bytes 10-1999/2000" ) == 0 );
    CHECK( range.getContentRangeString( 4, crs, 21 ) > 0);
    CHECK( strcmp( crs, "bytes 1999-1999/2000" ) == 0 );
    CHECK( range.getContentRangeString( 5, crs, 13 ) > 0);
    CHECK( strcmp( crs, "bytes */2000" ) == 0 );
    CHECK( range.getContentRangeString( 4, crs, 20 ) == -1);
    CHECK( range.getContentRangeString( 5, crs, 12 ) == -1);


    range.setContentLen( -1 );
    CHECK( range.getContentRangeString( 0, crs, 99 ) > 0);
    CHECK( strcmp( crs, "bytes 0-0/*" ) == 0 );
    CHECK( range.getContentRangeString( 1, crs, 99 ) > 0);
    CHECK( strcmp( crs, "bytes 1234-1999/*" ) == 0 );
    CHECK( range.getContentRangeString( 2, crs, 99 ) > 0);
    CHECK( strcmp( crs, "bytes 1990-1999/*" ) == 0 );
    CHECK( range.getContentRangeString( 3, crs, 99 ) > 0);
    CHECK( strcmp( crs, "bytes 10-1999/*" ) == 0 );
    CHECK( range.getContentRangeString( 4, crs, 38 ) > 0);
    CHECK( strcmp( crs, "bytes 1999-1999/*" ) == 0 );
    
    const char * psInSatified = "bytes=2000-2000,4423-123495";
    range.setContentLen( 2000 );
    CHECK( range.parse( psInSatified ) == SC_416 );
    const char * psSimple = "bytes=10-20";
    CHECK( range.parse( psSimple ) == 0 );
    
    HttpRange range1( el );
    range1.setContentLen( 2000 );
    const char * psBad1 = "bytes=--10";
    const char * psBad2 = "bytes=10--2345";
    const char * psBad3 = "bytes=10,";
    const char * psBad4 = "bytes=10 3-231";
    const char * psBad5 = "bytes=1-234 2";
    const char * psBad6 = "bytes=,1-2";
    const char * psBad7 = "bytes=-,1-2";
    const char * psBad8 = "bytes=1-2,,";
    const char * psBad9 = "bytes=1-2, 3 ,";
    const char * psBad10 = "bytes=1234-1123";
    CHECK( range1.parse( psBad1 ) == SC_400);
    CHECK( range1.parse( psBad2 ) == SC_400);
    CHECK( range1.parse( psBad3 ) == SC_400);
    CHECK( range1.parse( psBad4 ) == SC_400);
    CHECK( range1.parse( psBad5 ) == SC_400);
    CHECK( range1.parse( psBad6 ) == SC_400);
    CHECK( range1.parse( psBad7 ) == SC_400);
    CHECK( range1.parse( psBad8 ) == SC_400);
    CHECK( range1.parse( psBad9 ) == SC_400);
    CHECK( range1.parse( psBad10 ) == SC_400);

    HttpRange range2( 1447936 );
    const char * pdfRange =
    "bytes=1409802-1439801, 1439802-1447935, 3855-31023, "
    "31024-61023, 61024-91023, 91024-121023, 121024-151023, 151024-165400";
    int ret = range2.parse( pdfRange );
    CHECK( ret == 0 );
    
}

}


#endif
