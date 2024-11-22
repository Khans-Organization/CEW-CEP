#ifndef WEATHER_H
#define WEATHER_H

typedef struct {
    char city[100];
    char country[100];
    double temperature;
    int humidity;
} WeatherData;

int weatherData(const char *url, char *response);
int parseJSON(const char *json_data, WeatherData *data);
void writingInFile(const char *filename, const WeatherData *data);
void alertsChecking(const WeatherData *data);

#endif // WEATHER_H
