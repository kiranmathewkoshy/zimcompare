/*
 * Copyright (C)  Kiran Mathew Koshy
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */


/*
*This is a tool to compare two ZIM files.
*The two ZIM files are provided as command line arguments, and the difference is the output of the program.
*Usage: zimcompare file_1 file_2
*The tool prints missing articles, articles with different content, etc.
*/
#include <unistd.h>
#include <zim/file.h>
#include <getopt.h>
#include <zim/fileiterator.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <regex>
#include <ctime>
#include <cstdio>

class articleInfo
{
    public:
    std::string Title;
    std::string Data;
    int Hash;
    int index;

    bool operator <(articleInfo a)
    {
        return Title<a.Title?true:false;
    }
};

int djb2(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

int adler32(std::string buf)         //Adler32 Hash Function. Used to hash the BLOB data obtained from each article.
{
    unsigned int s1 = 1;
    unsigned int s2 = 0;
    int sz=buf.size();
    for (size_t n = 0; n <sz; n++)
    {
        s1 = (s1 + buf[n]) % 65521;
        s2 = (s2 + s1) % 65521;
    }
    return (s2 << 16) | s1;
}

int main(int argc, char **argv)
{

    //Parsing arguments
    //There will be only two arguments, so no detailed parsing is required.
    //The first argument is the
    if(argc<3)
    {
        std::cout<<"[ERROR] Not enough Arguments provided\n";
        return -1;
    }
    std::string filename_1 =argv[1];
    std::string filename_2 =argv[2];

    //Opening Files for reading

    try
    {
        zim::File file_1(filename_1);
        zim::File file_2(filename_2);

        //Data structure for storing article-related information
        std::list<articleInfo> data_1;
        std::list<articleInfo> data_2;
        data_1.clear();
        data_2.clear();

        //Scanning Data from files, generating hash
        zim::Blob tempBlob;      //Temporary data storage for storing the BLOB of an article.
        std::string tempString; //Temporary data storage, in oder to convart the blob
        articleInfo tempArticle; //Temporary data storage, in order to push to data_1 and data_2 lists.
        int counter=0;

        //Parsing through file_1
        for (zim::File::const_iterator it = file_1.begin(); it != file_1.end(); ++it)
        {
            tempBlob=it->getData();
            //tempString=tempBlob.data();
            tempArticle.Hash=djb2(tempBlob.data());
            //tempArticle.Hash=adler32(tempString);
            tempArticle.Data=tempBlob.data();
            tempArticle.Title=it->getTitle();
            tempArticle.index=counter;
            data_1.push_back(tempArticle);
            counter++;
        }

        counter=0;
        //Parsing through file_2
        for (zim::File::const_iterator it = file_2.begin(); it != file_2.end(); ++it)
        {
            tempBlob=it->getData();
            //tempString=tempBlob.data();
            //tempArticle.Hash=adler32(tempString);
            tempArticle.Hash=djb2(tempBlob.data());
            tempArticle.Title=it->getTitle();
            tempArticle.index=counter;
            data_2.push_back(tempArticle);
            counter++;
        }


        //Sorting the lists according to the title. This speeds up the linear search afterwards.
        data_1.sort();
        data_2.sort();


        //Comparing articles using the data obtained through parsing.
        std::list<articleInfo>::iterator prev=data_1.begin();
        for(std::list<articleInfo>::iterator it=++prev;it!=data_1.end();++it)
        {
            //if(it->Data==prev->Data)
                std::cout<<"\n"<<it->Hash<<" "<<it->Title;
        }


    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}























