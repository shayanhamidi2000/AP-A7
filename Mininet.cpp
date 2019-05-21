#include "Mininet.h"
#include "Config.h"
#include "Exceptions.h"
#include "Film.h"
#include <iostream>
#include <string>


MiniNet::MiniNet(){
	totalNetCredit = 0;
	theIdsAssigned = BASIC_ID_VALUE; 
	onlineUser = nullptr;
	manageCommand = new CommandHandler(this);
	systemSecurity = new Security();
	films = new FilmRepository();
}

MiniNet::~MiniNet(){
	delete manageCommand;
	delete systemSecurity;
	films->~FilmRepository();
	purchases.clear();
	users.clear();
}


void MiniNet::addToNetCredit(unsigned int amount){
	totalNetCredit += amount;
}

void MiniNet::withdrawNetCredit(unsigned int amount){
	totalNetCredit -= amount;
}

bool MiniNet::isOnlineUserPublisher() { 
	if(onlineUser == nullptr)
		return true;
	return onlineUser->getPublishingState();
}

bool MiniNet::isAnyOneOnline() {
	if(onlineUser != nullptr)
		return true;
	return false;
}

void MiniNet::goToNextId() {
 	theIdsAssigned++; 
}

void MiniNet::startNet(){
	string line = "";
	while(getline(cin , line)){
		try{
			manageCommand->getRawCommand(line);
		}catch(exception& ex){
			cout << ex.what() << endl;
		}
	}
}

void MiniNet::registerUser(string email , string username , string password , unsigned int age , bool isPublisher){
	if(isAnyOneOnline() )
		throw BadRequestException();
	systemSecurity->checkUsernameRepetition(users , username);
	if(isPublisher)
		onlineUser = new Publisher(username , systemSecurity->hashPassword(password , username) , email , theIdsAssigned , age);
	else
		onlineUser = new Customer(username , systemSecurity->hashPassword(password , username) , email , theIdsAssigned , age);

	users.push_back(onlineUser);
	this->goToNextId();
	cout << SUCCESS_MESSAGE << endl;
}

void MiniNet::loginUser(string username , string password){
	if(isAnyOneOnline() )
		throw BadRequestException();
	systemSecurity->isUsernameExisted(users , username);
	systemSecurity->isUsernameMatchingPassword(users , username , password);
	this->onlineUser = systemSecurity->findUserByUsername(users , username);

	cout << SUCCESS_MESSAGE << endl;
}

void MiniNet::logout(){
	if(!isAnyOneOnline() )
		throw BadRequestException();
	onlineUser = nullptr;
	cout << SUCCESS_MESSAGE << endl;
}


void MiniNet::addFilmOnNet(string name , unsigned int year , string director , string summary , unsigned int price , unsigned int length){
	if(!isAnyOneOnline() || !isOnlineUserPublisher())
		throw PermissionDenialException();

	films->addNewFilm((Publisher*) this->onlineUser , name , year , director , summary , price , length);
	cout << SUCCESS_MESSAGE << endl;
}

void MiniNet::editAFilm(unsigned int id , string newName , unsigned int newYear , unsigned int newLength , string newSummary , string newDirector){
	if(!isAnyOneOnline() || !isOnlineUserPublisher())
		throw PermissionDenialException();

	films->editFilm((Publisher*) this->onlineUser , id , newName , newYear , newLength , newSummary , newDirector);
	cout << SUCCESS_MESSAGE << endl;
}

void MiniNet::deleteAFilm(unsigned int id){
	if(!isAnyOneOnline() || !isOnlineUserPublisher())
		throw PermissionDenialException();

	films->deleteFilm((Publisher*) this->onlineUser , id );
	cout << SUCCESS_MESSAGE << endl;
}

void MiniNet::getPublishedList(string name , unsigned int minPoint , unsigned int minYear , unsigned int price , unsigned maxYear , string directorName){
	if(!isAnyOneOnline() || !isOnlineUserPublisher())
		throw PermissionDenialException();

	films->getPublihsedOrPurchasedList( ((Publisher*) onlineUser)->getUploadedFilms() , name , minPoint , minYear , price , maxYear , directorName);
}

void MiniNet::getFollowersList(){
	if(!isAnyOneOnline() || !isOnlineUserPublisher())
		throw PermissionDenialException();

	((Publisher*) onlineUser)->printYourFollowers();
}

void MiniNet::follow(unsigned int id){
	if(!isAnyOneOnline())
		throw PermissionDenialException();

	systemSecurity->checkIdExistence(users , id);
	if(!systemSecurity->findUserById(users , id)->getPublishingState() )
		throw PermissionDenialException();

	Publisher* followed = (Publisher*) systemSecurity->findUserById(users , id);
	followed->addToFollowers(onlineUser);
	onlineUser->sendMessageToFollowedPublisher(followed);
	cout << SUCCESS_MESSAGE << endl;	
}

void MiniNet::addMoney(unsigned int amount){
	if(!isAnyOneOnline() )
		throw PermissionDenialException();
	onlineUser->addToCredit(amount);
	cout << SUCCESS_MESSAGE << endl;
}

void MiniNet::buyFilm(unsigned int filmId){
	if(!isAnyOneOnline() )
		throw PermissionDenialException();

	Film* desiredFilm = films->findFilmByIdInDatabase(filmId);

	if(!onlineUser->hasFilm(desiredFilm) ){
		onlineUser->buyNewFilm(desiredFilm);
		addToNetCredit(desiredFilm->getPrice() );
		purchases.push_back(new Purchase(desiredFilm->getPrice() , desiredFilm->getRatingQuality() , desiredFilm->getOwner() ) );
	}
	cout << SUCCESS_MESSAGE << endl;
}

unsigned int MiniNet::getPublisherSoldFilmsMoney(){
	double earnedMoney = 0;
	vector<unsigned int> publisherPurchases;
	for(unsigned int i = 0 ; i < purchases.size() ; i++){
		if(purchases[i]->getFilmOwner() == this->onlineUser){
			earnedMoney += purchases[i]->calculateFilmOwnerShare();
			publisherPurchases.push_back(i);
		}
	}
	withdrawNetCredit((int) earnedMoney);
	deleteOverduedPurchases(publisherPurchases);
	return (int) earnedMoney;
}

void MiniNet::deleteOverduedPurchases(vector<unsigned int> overduedPurchases){
	for(unsigned int i = 0 ; i < overduedPurchases.size() ; i++){
		delete purchases[overduedPurchases[i] ];
		purchases.erase(purchases.begin() + overduedPurchases[i] );
		overduedPurchases = decreaseEachIndexByOne(overduedPurchases);
	}
}

vector<unsigned int> MiniNet::decreaseEachIndexByOne(vector<unsigned int> anIndexVector){
	vector<unsigned int> modifiedVector = anIndexVector;
	for(unsigned int i = 0 ; i < modifiedVector.size() ; i++)
		modifiedVector[i]--;

	return modifiedVector;
}

void MiniNet::getMoneyFromNet(){
	if(!isAnyOneOnline() || !isOnlineUserPublisher())
		throw PermissionDenialException();

	onlineUser->addToCredit(getPublisherSoldFilmsMoney() );
	cout << SUCCESS_MESSAGE << endl;
}

void MiniNet::getPurchasedList(string name , unsigned int minYear , unsigned int price , unsigned maxYear , string directorName){
	if(!isAnyOneOnline() )
		throw PermissionDenialException();

	films->getPublihsedOrPurchasedList( onlineUser->getPurchasedFilms() , name , NOT_A_FACTOR , minYear , price , maxYear , directorName);
}

void MiniNet::rateFilm(unsigned int filmId , unsigned int score){
	if(!isAnyOneOnline() )
		throw PermissionDenialException();

	films->checkFilmPurchased(onlineUser , filmId);
	Film* desiredFilm = films->findFilmByIdInDatabase(filmId);
	desiredFilm->rate(onlineUser , score);
	onlineUser->sendMessageToRatedPublisher(desiredFilm);
	cout << SUCCESS_MESSAGE << endl;
}

void MiniNet::comment(unsigned int filmId , string commentContent){
	if(!isAnyOneOnline() )
		throw PermissionDenialException();

	films->checkFilmPurchased(onlineUser , filmId);
	Film* desiredFilm = films->findFilmByIdInDatabase(filmId);
	desiredFilm->newComment(onlineUser , commentContent);
	onlineUser->sendMessageToCommentedPublisher(desiredFilm);
	cout << SUCCESS_MESSAGE << endl;
}

void MiniNet::replyComment(unsigned int filmId , unsigned int commentId , string content){
	if(!isAnyOneOnline() || !isOnlineUserPublisher())
		throw PermissionDenialException();

	films->checkFilmOwnership( (Publisher*)onlineUser , filmId);
	Film* desiredFilm = films->findFilmByIdInDatabase(filmId);
	desiredFilm->replyOneComment(commentId , content);
	((Publisher*) onlineUser)->notifyCommenterOnReply(desiredFilm->findCommentById(commentId)->getCommentOwner() );
	cout << SUCCESS_MESSAGE << endl;
}

void MiniNet::deleteComment(unsigned int filmId , unsigned int commentId){
	if(!isAnyOneOnline() || !isOnlineUserPublisher())
		throw PermissionDenialException();

	films->checkFilmOwnership( (Publisher*) onlineUser , filmId);
	Film* desiredFilm = films->findFilmByIdInDatabase(filmId);
	desiredFilm->deleteOneComment(commentId);
	cout << SUCCESS_MESSAGE << endl;
}

void MiniNet::getUnreadMessages(){
	if(!isAnyOneOnline() )
		throw PermissionDenialException();

	onlineUser->showUnreadMessages();
}

void MiniNet::getAllMessages(unsigned int limit){
	if(!isAnyOneOnline() )
		throw PermissionDenialException();

	onlineUser->showReadMessages(limit);
}

void MiniNet::searchFilmsInDatabase(string name , unsigned int minPoint , unsigned int minYear , unsigned int price , unsigned maxYear , string directorName){
	if(!isAnyOneOnline() )
		throw PermissionDenialException();

	films->getSearchedDatabaseList(name , minPoint , minYear , price , maxYear , directorName);
}

void MiniNet::showFurtherInfo(unsigned int filmId){
	if(!isAnyOneOnline() )
		throw PermissionDenialException();

	Film* desiredFilm = films->findFilmByIdInDatabase(filmId);
	desiredFilm->printDetailedVersionOfYourself();
	cout << endl;
	films->giveRecommendation(onlineUser , desiredFilm);
}