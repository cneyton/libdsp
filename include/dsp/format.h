#pragma once

#include <armadillo>

enum class Contract {
    unsupported_format,
    supported_format,
};

/**
 * @brief Format of a chunck
 */
struct Format
{
    arma::uword n_rows   = 0;
    arma::uword n_cols   = 0;
    arma::uword n_slices = 0;
};

inline
bool operator==(const Format& lhs, const Format& rhs)
{
    return (lhs.n_rows == rhs.n_rows) &&
        (lhs.n_cols == rhs.n_cols) &&
        (lhs.n_slices == rhs.n_slices);
}

inline
bool operator!=(const Format& lhs, const Format& rhs)
{
    return !(lhs == rhs);
}
