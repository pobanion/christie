#include <iostream>
#include <map>
#include <cmath>
#include "include/HTTPRequest.hpp" 	//https://github.com/elnormous/HTTPRequest/tree/master
#include "json/json.h"				//https://github.com/open-source-parsers/jsoncpp
using namespace std;

const string URL = "http://3.128.82.255:8000/items"; //AWS read items from DB


class Products {
    //string name;
	public:
    	double price;
    	double qty_price;
    	int discount_qty;

    	Products(){};
    	Products(double p, double q_p, int d_q){
    		price = p;
    		qty_price = q_p;
    		discount_qty = d_q;
    	}
};

class Order {
    string product;
    int qty;
};

class Checkout {
  public:
    map<string, Products> items;    //all the available items and pricing info
    map<string, int> order;         //all items in the current order

    Checkout(string);
    double total(); 
    bool scan(string);
};

//constructor
//pass in json string of products from DB
//parse it and add it to class list of available items
//init current order
Checkout::Checkout(string json_pricing_rules){
    //parse the json item pricing rules string
	Json::Value root;
	Json::Reader reader;
	bool parsingSuccessful = reader.parse( json_pricing_rules, root );
	if ( !parsingSuccessful ) {
			cout << "Error parsing the string" << endl;
			exit(0);
	}


    //add the items list to the map
	for (Json::Value::ArrayIndex i = 0; i != root.size(); i++)
	    if (root[i].isMember("id"))
	    	this->items.insert(pair<string, Products>(root[i]["product_name"].asString(), Products(root[i]["price"].asDouble(),
	    																								root[i]["qty_price"].asDouble(),
																										root[i]["discount_qty"].asInt())));

	//cout << this->items["apples"].price<< '\n';

}

double Checkout::total(){

	//current test rules from AWS DB
	//[{"id":1,"product_name":"apples","price":0.5,"qty_price":0.9,"discount_qty":3},
	//{"id":2,"product_name":"bananas","price":0.75,"qty_price":6,"discount_qty":10},
	//{"id":3,"product_name":"coconuts","price":2,"qty_price":3,"discount_qty":2},
	//{"id":4,"product_name":"donuts","price":0.5,"qty_price":null,"discount_qty":null}]

	map<string, int>::iterator it;
	double sum = 0;

	for (it = this->order.begin(); it != this->order.end(); it++) {

		if (!this->items[it->first].discount_qty)
			sum += it->second * this->items[it->first].price;
		else
			sum += (int(it->second / this->items[it->first].discount_qty) * this->items[it->first].qty_price) + ((it->second % this->items[it->first].discount_qty) * this->items[it->first].price);

	}

	//return sum;
	return std::round(sum * 100.0) / 100.0;
}

//add the specified item to the order
bool Checkout::scan(string item){
    //test the items list to see if the scanned item is valid
    map<string, Products>::iterator items_it = this->items.find(item);
    if (items_it == this->items.end()) {
        return false;
    }

    //add the scanned item to the order

    // check to see if the item is already in the order
    map<string, int>::iterator order_it = this->order.find(item);

    if (order_it != this->order.end()) {
        this->order[item]++; // inc the counter
    }
    else{
        this->order[item] = 1; //add the item
    }

    return true;

}

int main() {

	string result; //save the returned pricing rules text

    //read from the URL to get the item list
    try {
        // create an http request.
    	// Use the URL for the AWS DB "items"
        http::Request request{URL};

        // send the GET request and save the response
        const auto response = request.send("GET");

        //save the result
        result.append(response.body.begin(), response.body.end());

        // print the result
        //std::cout << std::string{response.body.begin(), response.body.end()} << '\n';

    }
    catch (const std::exception& e) {
        std::cerr << "Request failed, error: " << e.what() << '\n';
    }

    //create the checkout object with the current pricing rules received from the AWS DB (as a JSON string)
    Checkout co(result);

    co.scan("apples");
    co.scan("apples");
    co.scan("apples");

    co.scan("bananas");
    co.scan("bananas");

    co.scan("coconuts");
    co.scan("coconuts");

    co.scan("donuts");

    co.scan("apples");


    double price = co.total();
    std::cout << price << '\n';




}
