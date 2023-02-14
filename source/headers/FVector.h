#include <iostream>
#include <map>
#include <math.h>

#include "boost/json.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

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
            FVector(const string& serialized_fvector)
            {
                boost::json::value fvector = boost::json::parse(serialized_fvector);
                _size = fvector.at("size").as_int64();
                boost::json::object sparse_vector = fvector.at("vector").as_object();
                
                for (auto& it: sparse_vector)
                {
                    _sparse_vector[stoi(it.key().data())] = it.value().as_int64();
                }
            }

            map<size_t, size_t> get_sparse_vector() const
            {
                return _sparse_vector;
            }
            size_t size() const
            {
                return _size;
            }
            /*
                Makes serialization of existing FVector object to string with JSON format
            */
            string serialize() const
            {
                boost::json::object fvector;
                fvector["size"] = _size;

                boost ::json::object sparse_vector;
                for (auto& it: _sparse_vector)
                    sparse_vector[to_string(it.first)] = it.second;
                
                fvector["vector"] = sparse_vector;

                std::stringstream resstream;
                resstream << boost::json::serialize(fvector);

                return resstream.str();
            }

            void deserialize(const string& serialized_fvector)
            {
                _sparse_vector.clear();

                boost::json::value fvector = boost::json::parse(serialized_fvector);
                _size = fvector.at("size").as_int64();
                boost::json::object sparse_vector = fvector.at("vector").as_object();
                
                for (auto& it: sparse_vector)
                {
                    _sparse_vector[stoi(it.key().data())] = it.value().as_int64();
                }
            }

            friend double correlation(const FVector& compared_vector1, const FVector& compared_vector2);
            friend double euclidean_distance(const FVector& compared_vector1, const FVector& compared_vector2);
            friend double normalized_euclidean_distance(const FVector& compared_vector1, const FVector& compared_vector2);
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

        map<size_t, size_t> v1 = compared_vector1._sparse_vector;
        map<size_t, size_t> v2 = compared_vector2._sparse_vector;
        for (auto& it: v1)
        {
            double V1_i = (double)it.second;
            double V2_i = 0;
            try 
            {
                V2_i = (double)v2.at(it.first);
                v2.erase(it.first);
            }
            catch(const out_of_range&){ V2_i = 0;}

            divisible += ((V1_i - V1_hat) * (V2_i - V2_hat));
            divisor1 += (V1_i - V1_hat)*(V1_i - V1_hat);
            divisor2 += (V2_i - V2_hat)*(V2_i - V2_hat);
        }

        for (auto& it: v2)
        {
            double V1_i = 0;
            double V2_i = (double)it.second;
            try 
            {
                V1_i = (double)v1.at(it.first);
                v1.erase(it.first);
            }
            catch(const out_of_range&){ V1_i = 0;}

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

        map<size_t, size_t> v1 = compared_vector1._sparse_vector;
        map<size_t, size_t> v2 = compared_vector2._sparse_vector;
        for (auto& it: v1)
        {
            double V1_i = (double)it.second;
            double V2_i = 0;
            try 
            {
                V2_i = (double)v2.at(it.first);
                v2.erase(it.first);
            }
            catch(const out_of_range&){ V2_i = 0;}

            sum += ((V1_i - V2_i)*(V1_i - V2_i));
        }

        for (auto& it: v2)
        {
            double V1_i = 0;
            double V2_i = (double)it.second;
            try 
            {
                V1_i = (double)v1.at(it.first);
                v1.erase(it.first);
            }
            catch(const out_of_range&){ V1_i = 0;}

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
        Calculates the intersection between two given vectors
    */
    double intersection(const FVector& compared_vector1, const FVector& compared_vector2)
    {
        if (compared_vector1._size != compared_vector2._size)
            throw "Different size of vectors";

        double sum = 0;

        map<size_t, size_t> v1 = compared_vector1._sparse_vector;
        map<size_t, size_t> v2 = compared_vector2._sparse_vector;
        for (auto& it: v1)
        {
            double V1_i = (double)it.second;
            double V2_i = 0;
            try 
            {
                V2_i = (double)v2.at(it.first);
                v2.erase(it.first);
            }
            catch(const out_of_range&){ V2_i = 0;}

            sum += min(V1_i, V2_i);
        }

        for (auto& it: v2)
        {
            double V1_i = 0;
            double V2_i = (double)it.second;
            try 
            {
                V1_i = (double)v1.at(it.first);
                v1.erase(it.first);
            }
            catch(const out_of_range&){ V1_i = 0;}

            sum += min(V1_i, V2_i);
        }
        return sqrt(sum);
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

        map<size_t, size_t> v1 = compared_vector1._sparse_vector;
        map<size_t, size_t> v2 = compared_vector2._sparse_vector;
        for (auto& it: v1)
        {
            double V1_i = (double)it.second;
            double V2_i = 0;
            try 
            {
                V2_i = (double)v2.at(it.first);
                v2.erase(it.first);
            }
            catch(const out_of_range&){ V2_i = 0;}

            sum += min(V1_i, V2_i);
            sum1 += V1_i;
            sum += V2_i;
        }

        for (auto& it: v2)
        {
            double V1_i = 0;
            double V2_i = (double)it.second;
            try 
            {
                V1_i = (double)v1.at(it.first);
                v1.erase(it.first);
            }
            catch(const out_of_range&){ V1_i = 0;}

            sum += min(V1_i, V2_i);
            sum1 += V1_i;
            sum += V2_i;
        }

        return sum/max(sum1, sum2);
    }
}
 