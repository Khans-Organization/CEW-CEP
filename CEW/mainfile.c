#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <syslog.h> // For logging to syslog
#include <unistd.h> // For system calls like notify-send
#include "header_file.h" // Include your header file

// Callback function for writing the API response to a string
size_t Callback(void *cont, size_t size, size_t nmemb, void *userp) {
    ((char *)userp)[size * nmemb] = '\0'; // Null-terminate the received string
    strcat((char *)userp, cont);
    return size * nmemb;
}

// Function to parse JSON data and populate the WeatherData struct
int parseJSON(const char *json_data, WeatherData *data) {
    struct json_object *parsed_json;
    struct json_object *main;
    struct json_object *city;
    struct json_object *temp;
    struct json_object *humid;

    parsed_json = json_tokener_parse(json_data);
    if (parsed_json == NULL) {
        fprintf(stderr, "Parsing JSON Error\n");
        return 1;
    }

    main = json_object_object_get(parsed_json, "main");
    city = json_object_object_get(parsed_json, "name");
    temp = json_object_object_get(main, "temp");
    humid = json_object_object_get(main, "humidity");

    strcpy(data->city, json_object_get_string(city));
    data->temperature = json_object_get_double(temp);
    data->humidity = json_object_get_int(humid);

    // Print parsed data for debugging
    printf("City: %s\n", data->city);
    printf("Temperature: %.2f°C\n", data->temperature);
    printf("Humidity: %d%%\n", data->humidity);

    return 0;
}

// Function to fetch weather data using CURL
int weatherData(const char *url, char *response) {
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        fprintf(stderr, "Error initializing curl\n");
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Curl error: %s\n", curl_easy_strerror(res));
        return 1;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    // Print received data for debugging
    printf("Received Data: %s\n", response);
    return 0;
}

// Function to check and generate alerts
void alertsChecking(const WeatherData *data) {
    openlog("WeatherAlerts", LOG_PID | LOG_CONS, LOG_USER); // Open syslog session

    // Example alerts
    if (data->temperature > 35.0) {
        syslog(LOG_WARNING, "High temperature detected in %s: %.2f°C", data->city, data->temperature);
        system("notify-send 'Weather Alert' 'High temperature detected!'");
    }
    if (data->humidity < 20) {
        syslog(LOG_WARNING, "Low humidity detected in %s: %d%%", data->city, data->humidity);
        system("notify-send 'Weather Alert' 'Low humidity detected!'");
    }
    if (data->humidity > 80) {
        syslog(LOG_WARNING, "High humidity detected in %s: %d%%", data->city, data->humidity);
        system("notify-send 'Weather Alert' 'High humidity detected!'");
    }

    closelog(); // Close the syslog session
}

// Function to write weather data to a file
void writingInFile(const char *filename, const WeatherData *data) {
    FILE *file = fopen(filename, "a");
    if (file != NULL) {
        fprintf(file, "City: %s, Temperature: %.2f°C, Humidity: %d%%\n",
                data->city, data->temperature, data->humidity);
        fclose(file);
    } else {
        perror("Error opening file");
    }
}

int main() {
    char response[2048] = "";  // Buffer to store API response
    WeatherData data;

    // API URL with your OpenWeather API key
    const char *url = "http://api.openweathermap.org/data/2.5/weather?q=Karachi,PK&units=metric&appid=833e18e3d3df702326c6f5e1b57b3701";

    if (weatherData(url, response) == 0) {
        if (parseJSON(response, &data) == 0) {
            writingInFile("weather_data.txt", &data);  // Save data to file
            alertsChecking(&data);  // Check for alerts
        } else {
            fprintf(stderr, "Error parsing JSON data\n");
        }
    } else {
        fprintf(stderr, "Error fetching data from API\n");
    }

    return 0;
}
