#include "indexing.h"

using namespace std;
using namespace feature_vector;

int main(int argc, char* argv[])
{
    //time_t start = time(NULL);
    //INDEX DIRECTORY BY CONTENT
    // time_t start = time(NULL);
    indexing::index_directory_by_content("C:\\files\\kursa4\\test documents", "..\\..\\index.db");
    // time_t time_end = time(NULL) - start;
    // cout << "Indexed in " << time_end << " seconds" << endl;
    
    //FIND BY CONTENT IN DIRECTORY
    // while(1)
    // {
    //     cout << "Enter your request: ";
    //     string request;
    //     getline(cin, request);
    //     vector<string> found = indexing::knn_algorithm(request, "..\\..\\index.db", en_index_maker, 4);
    //     cout << found.size() << endl;
    //     for (auto& file: found)
    //         cout << file << endl;
    // }

    //INDEX DIRECTORY
    // cout << "Indexing Samples" << endl;
    // indexing::index_directory_to_db("D:\\", "..\\..\\index.db");

    // FIND IN DIRECTORY:
    // while(true)
    // {
    //     cout << "Enter your request: ";
    //     string request;
    //     cin >> request;
    //     unordered_set<string> found = indexing::find_in_db(request, "..\\..\\index.db");
    //     for (auto& path: found)
    //         cout << path << endl;
    // }
    system("pause");
}
