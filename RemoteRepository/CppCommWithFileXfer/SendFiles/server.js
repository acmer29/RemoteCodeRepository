var express = require("express");
var fs = require("fs");

var app  = express();

app.use(express.static('public'));

app.get('/demo', function(req, res) {
    return res.sendFile(__dirname + "/public/" + "demo.html");
})

app.get('/users', function (req, res) {
    fs.readFile( __dirname + "/" + "records.json", 'utf8', function (err, data) {
        data = JSON.parse(data);
        return res.json(data);
    });
})

app.get('/users/:username', function (req, res) {
    var userName = req.params.username;
    fs.readFile( __dirname + "/" + "records.json", 'utf8', function (err, data) {
        data = JSON.parse(data);
        var users = data["users"];
        for(var item in users) {
            if(users[item].username == userName) {
                return res.json(users[item]);
            }
            else continue;
        }
        return res.status(404).send("Not found");
    });
})

app.post('/users/:username', function (req, res) {
    var userName = req.params.username;
    var displayName = '', department = '';
    var body = '', jsonStr;
    req.on('data', function (chunk) {
        body += chunk;
    });
    req.on('end', function () {
        jsonStr = JSON.parse(body);
        displayName = jsonStr.displayName;
        department = jsonStr.department;
        fs.readFile( __dirname + "/" + "records.json", 'utf8', function (err, data) {
            data = JSON.parse(data);
            var users = data["users"];
            for(var item in users) {
                if(users[item].username == userName) {
                    return res.status(409).send("Record exists");
                }
            }
            var record = {"username": userName, "displayName": displayName, "department": department};
            users.push(record);
            var newData = {"users": users};
            newData = JSON.stringify(newData);
            fs.writeFile(__dirname + "/" + "records.json", newData, 'utf8', function(err) {
                return res.send("Success");
            })
        });
    });
}) 

app.delete('/users/:username', function (req, res) {
    var userName = req.params.username;
    fs.readFile( __dirname + "/" + "records.json", 'utf8', function (err, data) {
        data = JSON.parse(data);
        var users = data["users"];
        var flag = -1;
        for(var item in users) {
            if(users[item].username == userName) {
                flag = item;
				break;
            }
        }
        if(flag == -1) return res.status(404).send("Not exists");
        else {
            users.splice(flag, 1)
            var newData = {"users": users};
            newData = JSON.stringify(newData);
            fs.writeFile(__dirname + "/" + "records.json", newData, 'utf8', function(err) {
                return res.send("Success");
            })
        }
        
    });
})

var server = app.listen(8080);
