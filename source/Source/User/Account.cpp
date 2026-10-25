/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2017 - Lugaru contributors (see AUTHORS file)

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Lugaru is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lugaru.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "User/Account.hpp"

#include "Platform/Platform.hpp"
#include "Utils/Folders.hpp"
#include "Utils/binio.h"

#include <fstream>
#include <iostream>
#include <string.h>

using namespace std;

extern bool devtools;

vector<Account> Account::accounts;
int Account::i_active = -1;

Account::Account(const string& name)
    : name(name)
    , campaignProgress()
{
    difficulty = 0;
    progress = 0;
    points = 0;
    memset(highscore, 0, sizeof(highscore));
    memset(fasttime, 0, sizeof(fasttime));
    memset(unlocked, 0, sizeof(unlocked));

    setCurrentCampaign("main");
}

Account::Account(FILE* tfile)
    : Account("")
{
    funpackf(tfile, "Bi", &difficulty);
    funpackf(tfile, "Bi", &progress);
    int nbCampaigns;
    funpackf(tfile, "Bi", &nbCampaigns);

    for (int k = 0; k < nbCampaigns; ++k) {
        string campaignName = "";
        int t;
        char c;
        funpackf(tfile, "Bi", &t);
        for (int j = 0; j < t; j++) {
            funpackf(tfile, "Bb", &c);
            campaignName.append(1, c);
        }
        float fscore, fhighscore;
        funpackf(tfile, "Bf", &(campaignProgress[campaignName].time));
        funpackf(tfile, "Bf", &(fscore));
        funpackf(tfile, "Bf", &(fhighscore));
        campaignProgress[campaignName].score = fscore;
        campaignProgress[campaignName].highscore = fhighscore;
        int campaignchoicesmade, campaignchoice;
        funpackf(tfile, "Bi", &campaignchoicesmade);
        for (int j = 0; j < campaignchoicesmade; j++) {
            funpackf(tfile, "Bi", &campaignchoice);
            if (campaignchoice >= 10) {
                campaignchoice = 0;
            }
            campaignProgress[campaignName].choices.push_back(campaignchoice);
        }
    }

    currentCampaign = "";
    int t;
    char c;
    funpackf(tfile, "Bi", &t);
    for (int i = 0; i < t; i++) {
        funpackf(tfile, "Bb", &c);
        currentCampaign.append(1, c);
    }

    funpackf(tfile, "Bf", &points);
    for (int i = 0; i < 50; i++) {
        float fscore;
        funpackf(tfile, "Bf", &(fscore));
        funpackf(tfile, "Bf", &(fasttime[i]));
    }
    for (int i = 0; i < 60; i++) {
        char unlockedChar = 0;
        funpackf(tfile, "Bb", &unlockedChar);
        unlocked[i] = (unlockedChar != 0);
    }
    funpackf(tfile, "Bi", &t);
    for (int i = 0; i < t; i++) {
        funpackf(tfile, "Bb", &c);
        name.append(1, c);
    }
}

void Account::setCurrentCampaign(const string& name)
{
    currentCampaign = name;
}

void Account::add(const string& name)
{
    accounts.emplace_back(name);
    i_active = accounts.size() - 1;
}

Account& Account::get(int i)
{
    return accounts.at(i);
}

int Account::getNbAccounts()
{
    return accounts.size();
}

bool Account::hasActive()
{
    return (i_active >= 0);
}

Account& Account::active()
{
    static Account dummy("Player");
    if (i_active < 0 || i_active >= (int)accounts.size()) {
        return dummy;
    }
    return accounts[i_active];
}

void Account::setActive(int i)
{
    if ((i >= 0) && (i < int(accounts.size()))) {
        i_active = i;
    } else {
        cerr << "Tried to set active account to " << i << " but there is not such account" << endl;
        i_active = -1;
    }
}

void Account::destroyActive()
{
    if ((i_active >= 0) && (i_active < int(accounts.size()))) {
        accounts.erase(accounts.begin() + i_active);
        i_active = -1;
    } else {
        cerr << "Tried to destroy active account " << i_active << " but there is not such account" << endl;
        i_active = -1;
    }
    if (accounts.empty()) {
#ifdef __EMSCRIPTEN__
        Folders::saveStringToStorage("lugaru_users", "");
#endif
    }
}

int Account::getDifficulty()
{
    return difficulty;
}

void Account::endGame()
{
    campaignProgress[currentCampaign].choices.clear();
    campaignProgress[currentCampaign].score = 0;
    campaignProgress[currentCampaign].time = 0;
}

void Account::winCampaignLevel(int choice, int score, float time)
{
    campaignProgress[currentCampaign].choices.push_back(choice);
    setCampaignScore(campaignProgress[currentCampaign].score + score);
    campaignProgress[currentCampaign].time = time;
}

void Account::winLevel(int level, int score, float time)
{
    if (!devtools) {
        if (score > highscore[level]) {
            highscore[level] = score;
        }
        if (time < fasttime[level] || fasttime[level] == 0) {
            fasttime[level] = time;
        }
    }
    if (progress < level + 1) {
        progress = level + 1;
    }
}

void Account::loadFile(string filename)
{
    accounts.clear();
    i_active = -1;
#ifdef __EMSCRIPTEN__
    std::string data = Folders::loadStringFromStorage("lugaru_users");
    if (!data.empty()) {
        size_t p1 = data.find('\n');
        std::string name = (p1 == std::string::npos) ? data : data.substr(0, p1);
        if (!name.empty()) {
            int diff = 0, prog = 0;
            if (p1 != std::string::npos) {
                size_t p2 = data.find('\n', p1+1);
                std::string sDiff = data.substr(p1+1, p2-p1-1);
                std::string sProg = (p2 == std::string::npos) ? "" : data.substr(p2+1);
                diff = (sDiff.empty()) ? 0 : std::atoi(sDiff.c_str());
                prog = (sProg.empty()) ? 0 : std::atoi(sProg.c_str());
            }
            accounts.emplace_back(name);
            i_active = 0;
            active().difficulty = diff;
            active().progress = prog;
            printf("Loaded profile '%s' (diff=%d prog=%d)\n", name.c_str(), diff, prog);
            return;
        }
    }
#endif
    FILE* tfile = fopen(filename.c_str(), "rb");
    if (tfile) {
        int numaccounts;
        int iactive;
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
}

void Account::saveFile(string filename)
{
    FILE* tfile;
    errno = 0;
    tfile = fopen(filename.c_str(), "wb");
#ifdef __EMSCRIPTEN__
    if (getNbAccounts() > 0) {
        std::string txt = accounts[0].getName();
        txt += "\n" + std::to_string(active().getDifficulty());
        txt += "\n" + std::to_string(active().progress);
        Folders::saveStringToStorage("lugaru_users", txt);
    }
#endif
    if (tfile) {
        fpackf(tfile, "Bi", getNbAccounts());
        fpackf(tfile, "Bi", i_active);
        for (int i = 0; i < getNbAccounts(); i++) {
            printf("writing account %d/%d (%s)\n", i + 1, getNbAccounts(), accounts[i].getName().c_str());
            accounts[i].save(tfile);
        }
        fclose(tfile);
    } else {
        perror(("Couldn't save users in " + filename).c_str());
    }
}

void Account::save(FILE* tfile)
{
    fpackf(tfile, "Bi", difficulty);
    fpackf(tfile, "Bi", progress);
    int nbCampaigns = campaignProgress.size();
    fpackf(tfile, "Bi", nbCampaigns);
    for (map<string, CampaignProgress>::iterator it = campaignProgress.begin(); it != campaignProgress.end(); ++it) {
        const string& campaignName = it->first;
        fpackf(tfile, "Bi", (int)campaignName.size());
        for (unsigned int j = 0; j < campaignName.size(); j++) {
            char c = campaignName[j];
            fpackf(tfile, "Bb", (char)c);
        }
        float time = it->second.time;
        float fscore = it->second.score;
        float highscoreVal = it->second.highscore;
        float fasttimeVal = it->second.fasttime;
        fpackf(tfile, "Bf", &time);
        fpackf(tfile, "Bf", &(fscore));
        fpackf(tfile, "Bf", &(fasttimeVal));
        fpackf(tfile, "Bf", &(highscoreVal));
        // Not exactly same but good enough for persistence
        int campaignchoicesmade = it->second.choices.size();
        fpackf(tfile, "Bi", campaignchoicesmade);
        for (unsigned int j = 0; j < it->second.choices.size(); j++) {
            int choice = it->second.choices[j];
            fpackf(tfile, "Bi", choice);
        }
    }
    string current = currentCampaign;
    fpackf(tfile, "Bi", (int)current.size());
    for (unsigned int j = 0; j < current.size(); j++) {
        char c = current[j];
        fpackf(tfile, "Bb", (char)c);
    }
    fpackf(tfile, "Bf", &points);
    for (int i = 0; i < 50; i++) {
        float fhighscore = highscore[i];
        float ffasttime = fasttime[i];
        fpackf(tfile, "Bf", &fhighscore);
        fpackf(tfile, "Bf", &ffasttime);
    }
    for (int i = 0; i < 60; i++) {
        char unlockedChar = unlocked[i] ? 1 : 0;
        fpackf(tfile, "Bb", (char)unlockedChar);
    }
    fpackf(tfile, "Bi", (int)name.size());
    for (unsigned int j = 0; j < name.size(); j++) {
        char c = name[j];
        fpackf(tfile, "Bb", (char)c);
    }
}
