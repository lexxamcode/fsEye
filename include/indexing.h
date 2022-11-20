#include "boost/json.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/filesystem.hpp"
#include "../source/headers/FVectorMaker.h"

namespace indexing
{
    using namespace std;
    using namespace feature_vector;

    void index_entire_directory(const string& dir, const FVectorMaker& vector_maker, boost::json::object& main_json_target)
    {
        for (const auto& file: boost::filesystem::directory_iterator(dir))
        {
            if (boost::filesystem::is_directory(file))
                index_entire_directory(file.path().string(), vector_maker, main_json_target);
        
            else
            {
                boost::json::object current_file_fvector;
                FVector features = vector_maker.make_feature_vector(file.path().string());
                for (auto& it: features.get_sparse_vector())
                {
                    current_file_fvector[to_string(it.first)] = it.second;
                }
                main_json_target[file.path().string()] = current_file_fvector;
                cout << file.path().string() << endl;
            }
        }
    }

    // Now read from index.json
}