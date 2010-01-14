#define BOOST_TEST_MODULE example
#include <cstdio>
#include <boost/test/included/unit_test.hpp>

#include <cuv_general.hpp>
#include <dev_dense_matrix.hpp>
#include <host_dense_matrix.hpp>
#include <vector_ops.hpp>
#include <matrix_ops.hpp>
#include <convolution_ops.hpp>
#include <timing.hpp>
#include <random.hpp>
#include <matrix_ops/rprop.hpp>
#include <convert.hpp>

using namespace cuv;

struct Fix{
	static const int c = 2;  // # patterns (images)
	static const int n = 64;  // image size
	static const int f = 16;   // # filters
	static const int p = 8;	   // pooling size
	static const int g = 8;    // filter size
	static const int k = n-g+1;// target image size
	static const int o = n/p;  // pooling output size
	dev_dense_matrix<float, row_major>  d_img,d_filter,d_dst,d_pooled;
	host_dense_matrix<float, row_major> h_img,h_filter,h_dst,h_pooled;
	Fix()
	:   d_img(c,n*n), d_filter(f,g*g), d_dst(c,f*k*k), d_pooled(c,o*o)
	,   h_img(c,n*n), h_filter(f,g*g), h_dst(c,f*k*k), h_pooled(c,o*o)
	{
		//MEASURE_TIME("warmup", apply_scalar_functor(v, SF_EXP), 100);
	}
	~Fix(){
	}
};


BOOST_FIXTURE_TEST_SUITE( s, Fix )

BOOST_AUTO_TEST_CASE( convolution )
{
	fill(d_dst, 0.0f);
	sequence(d_img);    apply_scalar_functor(d_img,   SF_MULT,0.001f);
	sequence(d_filter); apply_scalar_functor(d_filter,SF_MULT,0.001f);

	fill(h_dst, 0.0f);
	sequence(h_img);    apply_scalar_functor(h_img,   SF_MULT,0.001f);
	sequence(h_filter); apply_scalar_functor(h_filter,SF_MULT,0.001f);

	convolve(d_dst,d_img,d_filter);
	convolve(h_dst,h_img,h_filter);

	host_dense_matrix<float, row_major> dst2(d_dst.h(), d_dst.w());
	convert(dst2,d_dst);

	for(int i=0;i<d_dst.h();i++){
		for(int j=0;j<d_dst.w();j++){
			BOOST_CHECK_CLOSE( dst2(i,j), h_dst(i,j), 0.001 );
		}
	}
}

BOOST_AUTO_TEST_CASE( local_maximum )
{
	fill(d_dst, 0.0f);
	sequence(d_img);    apply_scalar_functor(d_img,   SF_MULT,0.001f);

	fill(h_dst, 0.0f);
	sequence(h_img);    apply_scalar_functor(h_img,   SF_MULT,0.001f);

	localMaximum(h_pooled, h_img, p);
	localMaximum(d_pooled, d_img, p);

	host_dense_matrix<float, row_major> pooled2(d_pooled.h(), d_pooled.w());
	convert(pooled2,d_pooled);

	for(int i=0;i<d_pooled.h();i++){
		for(int j=0;j<d_pooled.w();j++){
			BOOST_CHECK_CLOSE( pooled2(i,j), h_pooled(i,j), 0.001 );
		}
	}
}
}