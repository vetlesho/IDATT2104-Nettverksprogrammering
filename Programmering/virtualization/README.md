# Python Code Runner

En webapplikasjon som lar deg kjøre Python-kode i en sikker Docker-container.

## Tjenester

### Frontend
- URL: http://localhost:80
- Kjører: Nginx webserver i Docker
- Innhold: Webgrensesnitt for å skrive og kjøre Python-kode

### Backend
- URL: http://localhost:8080/execute
- Kjører: C++ server i Docker
- Funksjon: Mottar og kjører Python-kode, returnerer resultat

## Kjør applikasjonen

```bash
# Stopp eventuelle kjørende containere
docker compose down

# Bygg og start containerne
docker compose up --build
```

## Testing
1. Åpne http://localhost i nettleseren
2. Skriv Python-kode i tekstfeltet
3. Klikk "Run Code"
4. Se resultatet under knappen