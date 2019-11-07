#pragma once

template <typename T>
class VectorStack
{
	public :
		VectorStack(){};

		void push(T t){ ar_.push_back(t); }
		//T top(){ return *(ar_.end()-1); }

		T top(){ return *(ar_.begin()); }
		void pop(){ ar_.erase(ar_.begin()); }
		bool empty(){ return ar_.empty(); }
		UINT size(){ return ar_.size(); }
		void clear(){ ar_.clear(); }

		/*void reverse()
		{
			std::vector<T> x;
			UINT n = ar_.size();
			x.resize(n);
			for( UINT a =0; a < n; a++ ){x[a] = ar_[n-a-1];	}
			ar_ = x;
		}*/
		bool exists(T t)
		{
			for( auto& it : ar_ )
				if ( t == it ) return true;
			return false;
		}

	//protected :
		std::vector<T> ar_;
};


