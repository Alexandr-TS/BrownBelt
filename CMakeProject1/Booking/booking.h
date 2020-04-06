#pragma once

#include <iostream>

namespace RAII {

	template <typename Provider>
	class Booking {
	public:
		Booking(Provider* provider_ptr, int counter)
			: provider_ptr(provider_ptr)
			, counter(counter) {
		}
		
		Booking(Booking&& b) {
			provider_ptr = b.provider_ptr;
			b.provider_ptr = nullptr;
		}

		Booking(const Booking&) = delete;
		Booking& operator = (Booking&& b) {
			provider_ptr = b.provider_ptr;
			b.provider_ptr = nullptr;
			return *this;
		}
		Booking& operator = (const Booking&) = delete;

		~Booking() {
			if (provider_ptr) {
				provider_ptr->CancelOrComplete(*this);
			}
		}

	private:
		Provider* provider_ptr;
		int counter;
	};

}