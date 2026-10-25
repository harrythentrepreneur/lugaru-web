void Account::loadFile(string filename)
{
    accounts.clear();
    i_active = -1;
#ifdef __EMSCRIPTEN__
    std::string data = Folders::loadStringFromStorage("lugaru_users");
    if (!data.empty()) {
        std::istringstream ss(data);
        std::string name;
        int progress = 0;
        int difficulty = 0;
        std::getline(ss, name);
        if (!name.empty() && name.find(':') != std::string::npos) {
            name = name.substr(name.find(':') + 1);
        }
        // Create a basic account with saved name
        accounts.emplace_back(name.c_str());
        i_active = 0;
        // Simple load: just name is enough for profile persistence
        printf("Loaded profile '%s' from localStorage.\n", name.c_str());
        return;
    }
#endif
    FILE* tfile = fopen(filename.c_str(), "rb");
    if (tfile) {
        int numaccounts = 0;
        int iactive = -1;
        funpackf(tfile, "Bi", &numaccounts);
        funpackf(tfile, "Bi", &iactive);
        printf("Loading %d accounts\n", numaccounts);
        for (int i = 0; i < numaccounts; i++) {
            printf("Loading account %d/%d\n", i, numaccounts);
            accounts.emplace_back(tfile);
        }
        fclose(tfile);
        setActive(iactive);
    } else {
        perror(("Couldn't load users from " + filename).c_str());
        i_active = -1;
    }
    if (accounts.empty()) {
        // Don't auto-create; user must create in menu
        i_active = -1;
    }
}
