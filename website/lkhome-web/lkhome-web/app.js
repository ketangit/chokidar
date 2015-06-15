/**
 * Module dependencies.
 */
var express = require('express');
var routes = require('./routes');
var http = require('http');
var path = require('path');
var mqtt = require('mqtt');

// sqllite3
var fs = require("fs");
var file = "./data/sensor.db";
var exists = fs.existsSync(file);
if (!exists) {
    console.log("Creating DB file - " + file);
    fs.openSync(file, "w");
}
var sqlite3 = require("sqlite3").verbose();
var db = new sqlite3.Database(file);
db.serialize(function() {
    db.run("CREATE TABLE IF NOT EXISTS SENSOR_STATUS (type TEXT, status TEXT, datetime LONG)");
});

var app = express();

// all environments
app.set('port', process.env.PORT || 3000);
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');
app.use(express.favicon());
app.use(express.logger('dev'));
app.use(express.json());
app.use(express.urlencoded());
app.use(express.methodOverride());
app.use(app.router);
app.use(require('stylus').middleware(path.join(__dirname, 'public')));
app.use(express.static(path.join(__dirname, 'public')));
app.locals.moment = require('moment');

// development only
if ('development' == app.get('env')) {
    app.use(express.errorHandler());
}

app.get('/', routes.index);
//app.get('/sensor', routes.sensor);

app.get('/sensor', function (req, res) {
    //var statusJson = { "sensor" : [] };
    db.all("SELECT * FROM SENSOR_STATUS ORDER BY datetime DESC LIMIT 10", function(error, rows) {
        if (error !== null) {
            next(err);
        } else {
            var message = rows.length > 0 ? "Viewing sensor status" : "No data found for sensors";
            res.render('sensor', { title: 'Sensor Status', year: new Date().getFullYear(), message: message, rows: rows });
        }
    });
});

// mqtt code
var mqtt = require('mqtt');
var client = mqtt.connect('ws://code.oneechotech.com:9009');
client.on('connect', function () {
    client.subscribe('SENSOR/+/STATUS');
    console.log("connected to mqtt broker");
});
client.on('error', function (error) {
    console.log("Error: " + error.toString());
});
client.on('message', function (topic, message) {
    console.log(topic + " = " + message.toString());
    if (topic !== null && typeof(topic) !== "undefined") {
        var topics = topic.split('/');
        if (topics.length == 3) {
            var now = new Date();
            var insertStmt = "INSERT INTO SENSOR_STATUS (type, status, datetime) VALUES('"
                + topics[1] + "', '"
                + message.toString() + "', '"
                + now.getTime() + "')";
            //console.log(insertStmt);
            db.run(insertStmt, function(error) {
                if (error !== null) {
                    next(error);
                }
            });
        }
    }
});

http.createServer(app).listen(app.get('port'), function () {
    console.log('Express server listening on port ' + app.get('port'));
});
