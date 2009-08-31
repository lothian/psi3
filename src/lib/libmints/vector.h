#ifndef _psi_src_lib_libmints_vector_h
#define _psi_src_lib_libmints_vector_h

/*!
    \file libmints/vector.h
    \ingroup MINTS
*/

#include <cstdlib>
#include <cstdio>

#include <libmints/ref.h>

namespace psi {

class Vector {
protected:
    /// Vector data
    double **vector_;
    /// Number of irreps
    int nirreps_;
    /// Dimensions per irrep
    int *dimpi_;

    /// Allocates vector_
    void alloc();
    /// Releases vector_
    void release();

    /// Copies data to vector_
    void copy_from(double **);

public:
    /// Default constructor, zeros everything out
    Vector();
    /// Copy constructor
    Vector(const Vector& copy);
    /// Constructor, allocates memory
    Vector(int nirreps, int *dimpi);

    /// Destructor, frees memory
    ~Vector();

    void init(int nirreps, int *dimpi);
    
    /// Sets the vector_ to the data in vec
    void set(double *vec);
    /// Returns a single element value
    double get(int h, int m) {
        return vector_[h][m];
    }
    /// Sets a single element value
    void set(int h, int m, double val) {
        vector_[h][m] = val;
    }
    /// Returns a copy of the vector_
    double *to_block_vector();

    /// Returns the dimension array
    int *dimpi() const {
        return dimpi_;
    }
    /// Returns the number of irreps
    int nirreps() const {
        return nirreps_;
    }

    /// Prints the vector
    void print(FILE *);
    /// Copies rhs to this
    void copy(const Vector* rhs);

    friend class Matrix;
};

class SimpleVector
{
protected:
    /// Vector data
    double *vector_;
    /// Dimension of the vector
    int dim_;

    /// Allocate memory
    void alloc();
    /// Free memory
    void release();

    /// Copy data to this
    void copy_from(double *);

public:
    /// Default constructor, zeroes everything out
    SimpleVector();
    /// Copy constructor
    SimpleVector(const SimpleVector& copy);
    /// Constructor, creates the vector
    SimpleVector(int dim);

    /// Destructor, frees memory
    ~SimpleVector();

    /// Set vector_ to vec
    void set(double *vec);
    /// Returns an element value
    double get(int m) {
        return vector_[m];
    }
    /// Sets an element value
    void set(int m, double val) {
        vector_[m] = val;
    }
    /// Returns a copy of vector_
    double *to_block_vector();

    /// Returns the dimension of the vector
    int dim() const {
        return dim_;
    }

    double& operator[](int i) { return vector_[i]; }

    void operator=(const SimpleVector& x) {
        for (int i=0; i<dim_; ++i)
            vector_[i] = x.vector_[i];
    }

    /// Prints the vector
    void print(FILE *);
    /// Copy rhs to this
    void copy(const SimpleVector* rhs);

    friend class SimpleMatrix;
};

}

#endif
