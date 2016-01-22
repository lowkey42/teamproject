/**************************************************************************\
 * encodes strings as unique integers                                     *
 *                                               ___                      *
 *    /\/\   __ _  __ _ _ __  _   _ _ __ ___     /___\_ __  _   _ ___     *
 *   /    \ / _` |/ _` | '_ \| | | | '_ ` _ \   //  // '_ \| | | / __|    *
 *  / /\/\ \ (_| | (_| | | | | |_| | | | | | | / \_//| |_) | |_| \__ \    *
 *  \/    \/\__,_|\__, |_| |_|\__,_|_| |_| |_| \___/ | .__/ \__,_|___/    *
 *                |___/                              |_|                  *
 *                                                                        *
 * Copyright (c) 2014 Florian Oetke                                       *
 *                                                                        *
 *  This file is part of MagnumOpus and distributed under the MIT License *
 *  See LICENSE file for details.                                         *
\**************************************************************************/

#pragma once

#include <string>
#include <stdexcept>
#include <iosfwd>
#include <cmath>
#include <algorithm>

namespace mo {
namespace util {

	class Str_id {
			static constexpr char step = 28;
		public:
			static constexpr std::size_t max_length = 13;

		public:
			explicit Str_id(const std::string& str) : Str_id(str.c_str()) {}
			explicit constexpr Str_id(const char* str = "") : _id(0) {
				using namespace std::string_literals;

				for(std::size_t i=0; str[i]!=0; ++i) {
					if(str[i]=='_')
						_id = (_id*step) + 1;
					else if(str[i]>='a' && str[i]<='z')
						_id = (_id*step) + (str[i]-'a' + 2);
					else
						throw std::invalid_argument("Unexpected character '"s+str[i]+"' in string: "+str);

					if(i>=max_length) {
						throw std::invalid_argument("String is too long: "s+str);
					}
				}
			}

			auto str()const -> std::string {
				std::string r;

				auto id = _id;
				while(id) {
					auto nid = id/step;
					auto c = id-(nid*step);
					id=nid;

					if(c==1) {
						r+='_';
					} else {
						r+=c+'a'-2;
					}
				}

				std::reverse(r.begin(), r.end());
				return r;
			}

			constexpr bool operator==(const Str_id& rhs)const noexcept {
				return _id==rhs._id;
			}
			constexpr bool operator!=(const Str_id& rhs)const noexcept {
				return _id!=rhs._id;
			}
			constexpr bool operator< (const Str_id& rhs)const noexcept {
				return _id<rhs._id;
			}
			constexpr operator uint64_t()const noexcept {
				return _id;
			}

		private:
			uint64_t _id;
	};

	inline std::ostream& operator<<(std::ostream& s, const Str_id& id) {
		s<<id.str();
		return s;
	}

}
}

inline constexpr mo::util::Str_id operator "" _strid(const char* str, std::size_t) {
	return mo::util::Str_id(str);
}

namespace std {
	template <> struct hash<mo::util::Str_id> {
		constexpr size_t operator()(mo::util::Str_id id)const noexcept {
			return id;
		}
	};
}

#ifdef BUILD_SERIALIZER
#include <sf2/sf2.hpp>

namespace mo {
namespace util {

	inline void load(sf2::JsonDeserializer& s, Str_id& v) {
		std::string str;
		s.read_value(str);
		v = Str_id(str);
	}

	inline void save(sf2::JsonSerializer& s, const Str_id& v) {
		s.write_value(v.str());
	}


}
}

#endif
