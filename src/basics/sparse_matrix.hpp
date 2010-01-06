#ifndef __SPARSE_MATRIX_HPP__
#define __SPARSE_MATRIX_HPP__
#include <vector>
#include <map>
#include <iostream>
#include <vector.hpp>
#include <matrix.hpp>
#include <cuv_general.hpp>

namespace cuv{
	template<class __value_type, class __index_type, class __vec_type>
	class dia_matrix 
	:        public matrix<__value_type, __index_type>{
	  public:
		  typedef __vec_type vec_type;
		  typedef matrix<__value_type, __index_type> base_type;
		  typedef typename matrix<__value_type, __index_type>::value_type value_type;
		  typedef typename matrix<__value_type, __index_type>::index_type index_type;
		protected:
		  int m_num_dia;                        ///< number of diagonals stored
		  int m_stride;                         ///< how long the stored diagonals are
		  bool m_is_transposed;                 ///< whether current state of the matrix is "transposed", changes access patterns
		  vec_type* m_vec;                      ///< stores the actual data 
		  std::vector<int> m_offsets;           ///< stores the offsets of the diagonals
		  std::map<int,index_type> m_dia2off;   ///< maps a diagonal to an offset
		public:
			dia_matrix(const index_type& h, const index_type& w, const int& num_dia, const int& stride)
				: base_type(h,w)
				, m_num_dia(num_dia)
				, m_stride(stride)
				, m_is_transposed(false)
				, m_offsets(num_dia)
			{
				alloc();
			}
			~dia_matrix(){
				dealloc();
			}
			void dealloc(){
				if(m_vec)
					delete m_vec;
			}
			void alloc(){
				cuvAssert(m_stride >= this->h());
				cuvAssert(m_stride >= this->w());
				m_vec = new vec_type(m_stride * m_num_dia);
			}
			inline const vec_type* vec()const{ return m_vec; }
			inline       vec_type* vec()     { return m_vec; }
			void transpose() {
				for(std::vector<int>::iterator it=m_offsets.begin(); it!= m_offsets.end(); it++)
					*it = -*it;
				m_is_transposed ^= 1;
			}

			//*****************************
			// set/get offsets of diagonals
			//*****************************
			template<class T>
			void set_offsets(const std::vector<T>& v){
				m_offsets.clear();
				std::copy(v.begin(), v.end(), std::back_inserter(m_offsets));
				for(unsigned int i = 0; i<m_offsets.size(); ++i)
					m_dia2off[m_offsets[i]] = i;
			}
			inline void set_offset(const index_type& idx, const index_type& val){
				m_offsets[idx] = val;
				m_dia2off[val] = idx;
			}
			inline const index_type& get_offset(const index_type& idx, const index_type& val)const{
				return m_offsets[idx];
			}

			// ******************************
			// read/write access
			// ******************************
			value_type operator()(const index_type& i, const index_type& j)const{
				using namespace std;
				if(!m_is_transposed){
					int off = (int)i - (int)j;
					typename map<int,index_type>::const_iterator it = m_dia2off.find(off);
					if( it == m_dia2off.end() )
						return (value_type) 0;
					return (*m_vec)[ it->second * m_stride +j  ];
				}
				else{
					int off = (int)j - (int)i; // transposed -> opposite
					typename map<int,index_type>::const_iterator it = m_dia2off.find(off);
					if( it == m_dia2off.end() )
						return (value_type) 0;
					return (*m_vec)[ it->second * m_stride +i  ];
				}
			}
	};
}

#endif /* __SPARSE_MATRIX_HPP__ */

