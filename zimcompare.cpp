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
#include <list>
#include <ctime>

class articleInfo                   //Class to store the title, namespace and hash of a file for comparison.
{
    public:
    std::string Title;
    int Hash;
    char Namespace;

    //Comparison operator. Required for std::sort()
    bool operator <(articleInfo a)
    {
        return Title<a.Title?true:false;
    }
};

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
    //Time variables in order to obtain time difference.
    time_t startTime,endTime;
    double  timeDiffference;
    time(&startTime);


    //Parsing arguments
    //There will be only two arguments, so no detailed parsing is required.
    std::cout<<"zimcompare";
    if(argc<3)
    {
        std::cout<<"\n[ERROR] Not enough Arguments provided\n";
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

        //List of articles to be deleted
        std::list<articleInfo> delete_list;
        std::list<articleInfo> update_list;


        //Scanning Data from files, generating hash
        zim::Blob tempBlob;      //Temporary data storage for storing the BLOB of an article.
        std::string tempString; //Temporary data storage, in oder to convert the blob
        articleInfo tempArticle; //Temporary data storage, in order to push to data_1 and data_2 lists.
        int length=0;

        //Parsing through files, generating hashes.
        std::cout<<"\n[INFO] Parsing through input ZIM files.."<<std::flush;
        //Parsing through file_1
        std::cout<<"\n[INFO] Parsing through "<<filename_1<<std::flush;
        for (zim::File::const_iterator it = file_1.begin(); it != file_1.end(); ++it)
        {
            tempBlob=it->getData();
            length=tempBlob.size();
            tempString.clear();
            for(int i=0; i<length; i++)
                tempString+=tempBlob.data()[i];
            tempArticle.Hash= adler32(tempString);
            tempArticle.Title=it->getTitle();
            tempArticle.Namespace=it->getNamespace();
            data_1.push_back(tempArticle);
        }

        //Parsing through file_2
        std::cout<<"\n[INFO] Parsing through "<<filename_2<<std::flush;
        for (zim::File::const_iterator it = file_2.begin(); it != file_2.end(); ++it)
        {
            tempBlob=it->getData();
            length=tempBlob.size();
            tempString.clear();
            for(int i=0; i<length; i++)
                tempString+=tempBlob.data()[i];
            tempArticle.Hash= adler32(tempString);
            tempArticle.Title=it->getTitle();
            tempArticle.Namespace=it->getNamespace();
            data_2.push_back(tempArticle);
        }


        //Sorting the lists according to the title. This speeds up the linear search afterwards.
        data_1.sort();
        data_2.sort();


        //Comparing articles using the data obtained through parsing.
        std::cout<<"\n[INFO] Comparing articles.."<<std::flush;
        int counter=0;
        while(data_1.size()>0)
        {
            std::list<articleInfo>::iterator it=data_1.begin();
            //Search through data_2 for an article with the same Title.
            for(std::list<articleInfo>::iterator sub_it=data_2.begin();sub_it!=data_2.end();++sub_it)
            {
                if(it->Title==sub_it->Title)
                {
                    //A match for an article with the same title.
                    if(it->Hash==sub_it->Hash)
                    {
                        //The article content has been unchanged.
                        counter++;
                    }
                    else
                    {
                        //The article content has changed.
                        update_list.push_back(*it);
                    }
                    //Once an article has been found in data_2, it is deleted from data_2 since there is no more use for the article and since it
                    //speeds up the remaining computation.
                    data_2.erase(sub_it);
                    break;
                }
                else
                {
                    //No Match for the article
                    //Article has been deleted.
                    delete_list.push_back(*it);
                }
            }
            data_1.erase(it);
        }


        //After the above search is complete, the elements listed in data_2 are the elements that need to be added

        //Displaying output:
        std::cout<<"\n[INFO] "<<delete_list.size()<<" articles deleted in "<<filename_2<<std::flush;
        std::cout<<"\n[INFO] "<<update_list.size()<<" articles updated in "<<filename_2<<std::flush;
        std::cout<<"\n[INFO] "<<update_list.size()<<" articles updated in "<<filename_2<<std::flush;
        std::cout<<"\n[INFO] "<<counter<<" articles remained unchanged\n"<<std::flush;

        //Computing time difference.
        time(&endTime);
        timeDiffference=difftime(endTime,startTime);
        std::cout<<"\n[INFO]Total time taken by zimcompare: "<<timeDiffference<<" seconds.\n"<<std::flush;

    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}























