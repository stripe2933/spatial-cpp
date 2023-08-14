//
// Created by gomkyung2 on 2023/08/14.
//

#ifndef SPATIAL_MACROS_HPP
#define SPATIAL_MACROS_HPP

#ifndef NDEBUG
#define NOEXCEPT_IF_RELEASE
#else
#define NOEXCEPT_IF_RELEASE noexcept
#endif

#endif //SPATIAL_MACROS_HPP
