// -----< NSPFileNameToNSNFileName: Convert nameSpace_fileName to nameSpace::fileName >-----
inline std::string NSPFileNameToNSNFileName(const std::string& NSPFileName) {
    if (NSPFileName == "") return "";
    std::string result = NSPFileName;
    std::string::size_type toReplace = result.find_first_of("_");
    return result.replace(toReplace, 1, "::");
}

inline std::string NSNFileNameToNSPFileName(const std::string& NSNFileName) {
    if(NSNFileName == "") return "";
    std::string result = NSNFileName;
    std::string::size_type toReplace = result.find_first_of("::");
    // if(toReplace = std::string::npos) throw std::exception("NSNFileNameToNSPFileName: Invalid NSNFileName given.\n");
    return result.replace(toReplace, 2, "_");
}

bool isLoopExist(std::vector<std::vector<bool>> graph) {
    int num = graph.size(), current = -1;
    std::queue<int> queHelper;
    if (num == 0) return false;
    std::vector<int> depend(num);
    for (int i = 0; i < num; ++i) {
        for (int j = 0; j < num; ++j) {
            if (graph[j][i] == true) depend[i] += 1;
        }
        if (depend[i] == 0) queHelper.push(i);
    }
    while (queHelper.empty() == false) {
        int over = queHelper.front();
        depend[over] = -1;
        queHelper.pop();
        for (int i = 0; i < num; ++i) {
            depend[i] -= graph[over][i];
            graph[over][i] = false;
            if (depend[i] == 0) queHelper.push(i);
        }
    }
    for (int i = 0; i < num; ++i) {
        if (depend[i] == -1) num -= 1;
    }
    return num != 0;
}

int main () {
    int a = 1, b = 2, c = 4, d = 8;
    while(1) {
//        std::string fortest;
//        std::cin >> fortest;
////        std::cout << NSPFileNameToNSNFileName(fortest) << std::endl;
////        std::cout << NSNFileNameToNSPFileName(fortest) << std::endl;
//        std::regex check("D:/test/open/_test\\.txt\\.[0-9]*");
////        std::cout << filter(fortest) << std::endl;
//        std::cout << std::regex_match(fortest, check) << std::endl;
        int n, m;
        std::cin >> n >> m;
        std::vector<std::vector<bool>> graph(n);
        for(int i = 0; i < n; ++i) {
            graph[i].resize(m);
            for(int j = 0; j < m; ++j) {
                bool input;
                std::cin >> input;
                graph[i][j] = input;
            }
        }
        for(int i = 0; i < n; ++i) {
            for(int j = 0; j < m; ++j) {
                std::cout << graph[i][j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << isLoopExist(graph) << std::endl;
    }
    return 0;
}