#ifndef CPPEG_ASSERTIONS_HPP
#define CPPEG_ASSERTIONS_HPP

#include <cassert>
#include <iostream>

#ifndef NDEBUG
#define debug_assert(COND, MSG) \
    if(!(COND)) {  \
	std::cerr << "At " << __FILE__ << ":" << __LINE__ \
		  << ", condition " << STR(COND) << ": " << MSG << "\n"; \
    }
#else
#define debug_assert(COND,MSG)
#endif

#endif
