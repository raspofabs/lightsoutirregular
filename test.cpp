#include "util.h"

bool verbose = true;
int loggingLevel = 3;

#include "ConfigReader.h"

struct Workspace {
	typedef std::vector<int> ivec;
	typedef std::vector<ivec> ivecvec;
	ivecvec board;
};

void AddBoardLine( const char *line, void *user ) {
	Workspace *w = (Workspace*)user;
	ivec newRow;
	while( *line ) {
		float val;
		int r = sscanf( line, "%f", &val );
	}
	if( newRow.size() > 0 ) {
		w.board.push_back( newRow );
	}
}
void AddResult( const char *line, void *user ) {
	V3 v;
	sscanf( line, "%f,%f,%f", &v.x, &v.y, &v.z );
	Workspace *w = (Workspace*)user;
	w->results.push_back( v );
}
void SetupQuaternion( const char *line, void *user ) {
	Quat q;
	int num = sscanf( line, "%f,%f,%f,%f", &q.s, &q.i, &q.j, &q.k );
	if( num == 4 ) {
		Workspace *w = (Workspace*)user;
		w->quat = q;
		logf( 1, "Load Quaternion %.2f, %.2f,%.2f,%.2f\n", q.s, q.i,q.j,q.k );
		return;
	}
	num = sscanf( line, "%f*%f,%f,%f", &q.s, &q.i, &q.j, &q.k );
	if( num == 4 ) {
		Workspace *w = (Workspace*)user;
		logf( 1, "Load AxisAngle %.2f (%.2fpi), %.2f,%.2f,%.2f\n", q.s, q.s/M_PI, q.i,q.j,q.k );
		q = QuatFromAngleAxis(q.s,q.i,q.j,q.k);
		w->quat = q;
		return;
	}
}
LineReaderCallback GetCallbackFromHeader( const char *line, void * ) {
	if( 0 == strcasecmp( line, "verts" ) ) { return AddVertex; }
	if( 0 == strcasecmp( line, "results" ) ) { return AddResult; }
	if( 0 == strcasecmp( line, "quat" ) ) { return SetupQuaternion; }
	logf( 1, "Unexpected config header [%s], returning null handler\n", line );
	return 0;
}


#define TestAssert( X ) if( !( X ) ) { logf( 1, RED "TEST FAILED" CLEAR "[%s]\n", #X ); return false; } else { logf( 3, GREEN "TEST PASSED" CLEAR "[%s]\n", #X ); }

bool RunPuzzle( int testID ) {
	char filename[128];
	sprintf( filename, "test%i.txt", testID );
	Workspace w;
	int result = OpenConfigAndCallbackPerLine( filename, GetCallbackFromHeader, 0, &w );
	TestAssert( 0 == result );
	TestAssert( w.verts.size() > 0 );
	TestAssert( w.results.size() > 0 );
	//logf( 1, "Testing %i vs %i\n", w.verts.size(), w.results.size() );
	TestAssert( w.results.size() == w.verts.size() );
	Quat q = w.quat;
	Quat iq = conj(w.quat);
	logf( 1, "Testing multiplication by Q(%.2f,%.2f,%.2f,%.2f)\n", q.s,q.i,q.j,q.k );
	for( size_t i = 0; i < w.verts.size(); ++i ) {
		V3 in = w.verts[i];
		V3 result = w.results[i];
		logf( 1, "Testing Q(%.2f,%.2f,%.2f,%.2f) * V(%.2f,%.2f,%.2f) * Q(%.2f,%.2f,%.2f,%.2f) = V(%.2f,%.2f,%.2f)\n", q.s,q.i,q.j,q.k, in.x,in.y,in.z, iq.s,iq.i,iq.j,iq.k, result.x, result.y,result.z );
		Quat r = V3ToQuat( in );
		logf( 1, "r = %f,%f,%f,%f\n", r.s,r.i,r.j,r.k );
		Quat qr = q * r;
		logf( 1, "qr = %f,%f,%f,%f\n", qr.s,qr.i,qr.j,qr.k );
		Quat out = qr * iq;
		logf( 1, "out = %f,%f,%f,%f\n", out.s,out.i,out.j,out.k );
		V3 v = QuatToV3( out );
		logf( 1, "v = %f,%f,%f\n", v.x, v.y, v.z );
		TestAssert( 0.1 > fabs( v.x - result.x ) );
		TestAssert( 0.1 > fabs( v.y - result.y ) );
		TestAssert( 0.1 > fabs( v.z - result.z ) );
	}
	return true;
}

const int MAX_PUZZLE_ID = 1;

int main( ) {
	for( int tid = 1; tid <= MAX_PUZZLE_ID; ++tid ) {
		RunPuzzle(tid) );
	}

	logf( 1, GREEN "ALL PUZZLES SOLVED" CLEAR "\n" );

	return 0;
}
