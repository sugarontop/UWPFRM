#pragma once

// stack‚ðvector‚ÅŽÀ‘•

template <typename T>
class VectorStack
{
	public :
		VectorStack(){};

		void push(T t){ ar_.push_back(t); }
		T top(){ return *(ar_.end()-1); }
		void pop(){ ar_.erase( ar_.end()-1); }
		bool empty(){ return ar_.empty(); }
		UINT size(){ return ar_.size(); }

		std::vector<T> ar_;
};