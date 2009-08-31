#include <cstdlib>
#include <string.h>
#include "vector.h"

using namespace psi;

Vector::Vector() {
    vector_ = NULL;
    dimpi_ = NULL;
    nirreps_ = 0;
}

Vector::Vector(const Vector& c) {
    vector_ = NULL;
    nirreps_ = c.nirreps_;
    dimpi_ = new int[nirreps_];
    for (int h=0; h<nirreps_; ++h)
        dimpi_[h] = c.dimpi_[h];
    alloc();
    copy_from(c.vector_);
}

Vector::Vector(int nirreps, int *dimpi) {
    vector_ = NULL;
    nirreps_ = nirreps;
    dimpi_ = new int[nirreps_];
    for (int h=0; h<nirreps_; ++h)
        dimpi_[h] = dimpi[h];
    alloc();
}

Vector::~Vector() {
    release();
    if (dimpi_)
        delete[] dimpi_;
}

void Vector::init(int nirreps, int *dimpi)
{
    if (dimpi_) delete[] dimpi_;
    nirreps_ = nirreps;
    dimpi_ = new int[nirreps_];
    for (int h=0; h<nirreps_; ++h)
        dimpi_[h] = dimpi[h];
    alloc();
}

void Vector::alloc() {
    if (vector_)
        release();
    
    vector_ = (double**)malloc(sizeof(double*) * nirreps_);
    for (int h=0; h<nirreps_; ++h) {
        if (dimpi_[h])
            vector_[h] = new double[dimpi_[h]];
    }
}

void Vector::release() {
    if (!vector_)
        return;
    
    for (int h=0; h<nirreps_; ++h) {
        if (dimpi_[h])
            delete[] (vector_[h]);
    }
    free(vector_);
    vector_ = NULL;
}

void Vector::copy_from(double **c) {
    size_t size;
    for (int h=0; h<nirreps_; ++h) {
        size = dimpi_[h] * sizeof(double);
        if (size)
            memcpy(&(vector_[h][0]), &(c[h][0]), size);
    }
}

void Vector::copy(const Vector *rhs) {
    if (nirreps_ != rhs->nirreps_) {
        release();
        if (dimpi_)
            delete[] dimpi_;
        nirreps_ = rhs->nirreps_;
        dimpi_ = new int[nirreps_];
        for (int h=0; h<nirreps_; ++h)
            dimpi_[h] = rhs->dimpi_[h];
        alloc();
    }
    copy_from(rhs->vector_);
}

void Vector::set(double *vec) {
    int h, i, ij;
    
    ij = 0;
    for (h=0; h<nirreps_; ++h) {
        for (i=0; i<dimpi_[h]; ++i) {
            vector_[h][i] = vec[ij++];
        }
    }
}

void Vector::print(FILE *out) {
    int h;
    for (h=0; h<nirreps_; ++h) {
        fprintf(out, " Irrep: %d\n", h+1);
        for (int i=0; i<dimpi_[h]; ++i)
            fprintf(out, "   %4d: %10.7f\n", i+1, vector_[h][i]);
        fprintf(out, "\n");
    }
}

double *Vector::to_block_vector() {
    size_t size=0;
    for (int h=0; h<nirreps_; ++h)
        size += dimpi_[h];
    
    double *temp = new double[size];
    size_t offset = 0;
    for (int h=0; h<nirreps_; ++h) {
        for (int i=0; i<dimpi_[h]; ++i) {
            temp[i+offset] = vector_[h][i];
        }
        offset += dimpi_[h];
    }
    
    return temp;
}

//
// SimpleVector class
//
SimpleVector::SimpleVector() {
    vector_ = NULL;
    dim_ = 0;
}

SimpleVector::SimpleVector(const SimpleVector& c) {
    vector_ = NULL;
    dim_ = c.dim_;
    alloc();
    copy_from(c.vector_);
}

SimpleVector::SimpleVector(int dim) {
    vector_ = NULL;
    dim_ = dim;
    alloc();
}

SimpleVector::~SimpleVector() {
    release();
}

void SimpleVector::alloc() {
    if (vector_)
        release();
    
    vector_ = new double[dim_];
    memset(vector_, 0, sizeof(double) * dim_);
}

void SimpleVector::release() {
    if (!vector_)
        return;
    
    delete[] (vector_);
    vector_ = NULL;
}

void SimpleVector::copy_from(double *c) {
    size_t size;
    size = dim_ * sizeof(double);
    if (size)
        memcpy(&(vector_[0]), &(c[0]), size);
}

void SimpleVector::copy(const SimpleVector *rhs) {
    release();
    dim_ = rhs->dim_;
    alloc();
    copy_from(rhs->vector_);
}

void SimpleVector::set(double *vec) {
    int i;
    
    for (i=0; i<dim_; ++i) {
        vector_[i] = vec[i];
    }
}

void SimpleVector::print(FILE *out) {
    for (int i=0; i<dim_; ++i)
        fprintf(out, "   %4d: %10.7f\n", i+1, vector_[i]);
    fprintf(out, "\n");
}

double *SimpleVector::to_block_vector() {
    double *temp = new double[dim_];
    for (int i=0; i<dim_; ++i) {
        temp[i] = vector_[i];
    }
    
    return temp;
}
