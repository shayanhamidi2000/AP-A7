#ifndef FILM_H
#define FILM_H

#include <string>
#include <vector>
#include "Comment.h"

class Publisher;

class Film{
public:
	Film(unsigned int id , std::string name , unsigned manufacturedYear , unsigned int length , 
		unsigned int price , std::string summary , std::string directorName , Publisher* filmOwner);

	bool getAvailability() const;
	void beUnavailable();
	unsigned int getId() const;
	double getPoint() const;
	void setName(std::string name);
	std::string getName() const;
	std::string getDirectorName() const;
	unsigned int getYear() const;
	void setYear(unsigned int manufacturedYear);
	void setDuration(unsigned int length);
	unsigned int getPrice() const;
	void setDirectorName(std::string directorName);
	void setSummary(std::string summary);
	Publisher* getOwner() const;
	void printYourself() const;
private:
	Publisher* filmOwner;
	int theIdsAssignedToComments;
	unsigned int id;
	bool isAvailable;
	double averagePoint;
	unsigned int peopleRatedThisFilm;
	std::string name;
	unsigned int manufacturedYear;
	unsigned int length;
	unsigned int price;
	std::string summary;
	std::string directorName;
	std::vector<Comment*> comments;
};

#endif