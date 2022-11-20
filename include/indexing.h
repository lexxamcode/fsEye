#include "boost/json.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/filesystem.hpp"
#include "../source/headers/FVectorMaker.h"

namespace indexing
{
    using namespace std;
    using namespace feature_vector;
    namespace pt = boost::property_tree;
    const std::regex json_file("(.*?)(\.json)");

    void index_entire_directory(const string& dir, const FVectorMaker& vector_maker, boost::json::object& main_json_target)
    {
        
        for (const auto& file: boost::filesystem::directory_iterator(dir))
        {
            if (boost::filesystem::is_directory(file))
                index_entire_directory(file.path().string(), vector_maker, main_json_target);
        
            else
            {
                boost::json::object current_file_fvector;
                FVector features = vector_maker.make_feature_vector(file.path().string(), 1);
                for (auto& it: features.get_sparse_vector())
                {
                    current_file_fvector[to_string(it.first)] = it.second;
                }
                main_json_target[file.path().string()] = current_file_fvector;
                cout << file.path().string() << endl;
            }
        }
    }

    map<string, FVector> read_from_json(const string& path_to_json_file, size_t dict_size)
    {
        if (!std::regex_match(path_to_json_file, json_file))
            throw "Not a .json file";

        map<string, FVector> result_data;

        pt::ptree load_ptree_root;
        try
        {
            pt::read_json(path_to_json_file, load_ptree_root);
        }
        catch (pt::json_parser::json_parser_error)
        {
            cout << "Error while reading .json file" << endl;
            return result_data;
        }

        for (auto& path: load_ptree_root)
        {
            cout << path.first << endl;
            map<size_t, size_t> sparse_vector;
            for (auto& index: path.second)
            {
                auto& value = index.second;
                sparse_vector[stoi(index.first)] = stoi(value.data());
            }
            FVector temp(sparse_vector, dict_size);
            result_data[path.first] = temp;
        }

        return result_data;
    }
    // Now read from index.json

    vector<string> knn_algorithm(const string& text, const string& path_to_json, const FVectorMaker& maker, size_t k)
    {
        FVector given_fvector = maker.make_feature_vector(text, 0);
        map<string, FVector> set_of_vectors = read_from_json(path_to_json, maker.get_dict_size());

        vector<pair<double, string>> distances;

        for (auto& it: set_of_vectors)
        {
            double distance = correlation(it.second, given_fvector);
            pair<double, string> dist_path(distance, it.first);
            distances.push_back(dist_path);
        }

        sort(distances.begin(), distances.end());

        vector<string> found;

        for (int i = distances.size() - 1; i >= distances.size() - k; i--)
        {
            found.push_back(distances[i].second);
        }
        return found;
    }
}