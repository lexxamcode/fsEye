#include <iostream>
#include <map>
#include <math.h>

using namespace std;

namespace feature_vector
{
    class FVector
    {
        private:
            /*
                Feature vector class is a sparse vector class
                made of map of indexes and their values, and also of
                _size parameter describing the size of vector
            */
            map<size_t, size_t> _sparse_vector; 
            size_t _size;

            double average() const
            {
                size_t sum = 0;
                for (auto &it: _sparse_vector)
                sum += it.second;
            
                return static_cast<double>((double)sum/_size);
            }
        public:
            FVector(): _size(0) {}
            FVector(size_t size): _size(size) { }
            FVector(const map<size_t, size_t>& sparse_vector, size_t size): _sparse_vector(sparse_vector), _size(size) { }

            map<size_t, size_t> get_sparse_vector() const
            {
                return _sparse_vector;
            }
            size_t size() const
            {
                return _size;
            }
            /*
                Calculates the correlation between two given vectors
            */
            double correlation(const FVector& compared_vector) const 
            {
                if (_size != compared_vector._size)
                    return 0;

                double V1_hat = average();
                double V2_hat = compared_vector.average();

                double divisible = 0;
                double divisor1 = 0;
                double divisor2 = 0;
                for (size_t i = 0; i < _size; i++)
                {
                    double V1_i = 0;
                    double V2_i = 0;

                    try {V1_i = (double)_sparse_vector.at(i);}
                    catch(const out_of_range&){ V1_i = 0;}
                    try {V2_i = (double)compared_vector._sparse_vector.at(i);}
                    catch(const out_of_range&){ V2_i = 0;}

                    divisible += ((V1_i - V1_hat) * (V2_i - V2_hat));
                    divisor1 += (V1_i - V1_hat)*(V1_i - V1_hat);
                    divisor2 += (V2_i - V2_hat)*(V2_i - V2_hat);
                }
                return divisible/(sqrt(divisor1*divisor2));
            }

            friend double correlation(const FVector& compared_vector1, const FVector& compared_vector2);
            friend double euclidean_distance(const FVector& compared_vector1, const FVector& compared_vector2);
            friend double normalized_euclidean_distance(const FVector& compared_vector1, const FVector& compared_vector2);
            friend double chi_square(const FVector& compared_vector1, const FVector& compared_vector2);
            friend double intersection(const FVector& compared_vector1, const FVector& compared_vector2);
            friend double normalized_intersection(const FVector& compared_vector1, const FVector& compared_vector2);
    };

    /*
        Calculates the correlation between two given vectors
    */
    double correlation(const FVector& compared_vector1, const FVector& compared_vector2)
    {   
        if (compared_vector1._size != compared_vector2._size)
            return 0;

        double V1_hat = compared_vector1.average();
        double V2_hat = compared_vector2.average();

        double divisible = 0;
        double divisor1 = 0;
        double divisor2 = 0;
        for (size_t i = 0; i < compared_vector1._size; i++)
        {
            double V1_i = 0;
            double V2_i = 0;

            try {V1_i = (double)compared_vector1._sparse_vector.at(i);}
            catch(const out_of_range&){ V1_i = 0;}
            try {V2_i = (double)compared_vector2._sparse_vector.at(i);}
            catch(const out_of_range&){ V2_i = 0;}

            divisible += ((V1_i - V1_hat) * (V2_i - V2_hat));
            divisor1 += (V1_i - V1_hat)*(V1_i - V1_hat);
            divisor2 += (V2_i - V2_hat)*(V2_i - V2_hat);
        }
        
        return divisible/(sqrt(divisor1*divisor2));
    }

    /*
        Calculates the euclidean distance between two given vectors
    */
    double euclidean_distance(const FVector& compared_vector1, const FVector& compared_vector2)
    {
        if (compared_vector1._size != compared_vector2._size)
           throw "Different size of vectors";
        
        double sum = 0;
        for (size_t i = 0; i < compared_vector1._size; i++)
        {
            double V1_i = 0;
            double V2_i = 0;

            try {V1_i = (double)compared_vector1._sparse_vector.at(i);}
            catch(const out_of_range&){ V1_i = 0;}
            try {V2_i = (double)compared_vector2._sparse_vector.at(i);}
            catch(const out_of_range&){ V2_i = 0;}

            sum += ((V1_i - V2_i)*(V1_i - V2_i));
        }
        return sqrt(sum);
    }
    /*
        Calculates normalized euclidean distance between two given vectors
    */
    double normalized_euclidean_distance(const FVector& compared_vector1, const FVector& compared_vector2)
    {
        if (compared_vector1._size != compared_vector2._size)
            throw "Different size of vectors";

        return euclidean_distance(compared_vector1, compared_vector2)/sqrt(compared_vector1._size);  
    }

    /*
        Calculates the chi square distance between two given vectors
    */
    double chi_square(const FVector& compared_vector1, const FVector& compared_vector2)
    {
        if (compared_vector1._size != compared_vector2._size)
           throw "Different size of vectors";
        
        double sum = 0;
        for (size_t i = 0; i < compared_vector1._size; i++)
        {
            double V1_i = 0;
            double V2_i = 0;

            try {V1_i = (double)compared_vector1._sparse_vector.at(i);}
            catch(const out_of_range&){ V1_i = 0;}
            try {V2_i = (double)compared_vector2._sparse_vector.at(i);}
            catch(const out_of_range&){ V2_i = 0;}

            sum += ((V1_i - V2_i)*(V1_i - V2_i)/V1_i);
        }
        return sum;
    }
    /*
        Calculates the intersection between two given vectors
    */
    double intersection(const FVector& compared_vector1, const FVector& compared_vector2)
    {
        if (compared_vector1._size != compared_vector2._size)
            throw "Different size of vectors";
        
        double sum = 0;
        for (size_t i = 0; i < compared_vector1._size; i++)
        {
            double V1_i = 0;
            double V2_i = 0;

            try {V1_i = (double)compared_vector1._sparse_vector.at(i);}
            catch(const out_of_range&){ V1_i = 0;}
            try {V2_i = (double)compared_vector2._sparse_vector.at(i);}
            catch(const out_of_range&){ V2_i = 0;}

            sum += min(V1_i, V2_i);
        }

        return sum;
    }

    /*
        Calculates normalized intersection between two given vectors
    */
    double normalized_intersection(const FVector& compared_vector1, const FVector& compared_vector2)
    {
        if (compared_vector1._size != compared_vector2._size)
            throw "Different size of vectors";
        
        double sum = 0;
        double sum1 = 0;
        double sum2 = 0;
        for (size_t i = 0; i < compared_vector1._size; i++)
        {
            double V1_i = 0;
            double V2_i = 0;

            try {V1_i = (double)compared_vector1._sparse_vector.at(i);}
            catch(const out_of_range&){ V1_i = 0;}
            try {V2_i = (double)compared_vector2._sparse_vector.at(i);}
            catch(const out_of_range&){ V2_i = 0;}

            sum += min(V1_i, V2_i);
            sum1 += V1_i;
            sum += V2_i;
        }

        return sum/max(sum1, sum2);
    }
}
