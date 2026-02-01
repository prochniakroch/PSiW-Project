Roch Próchniak 2025 

1. Kompilacja 
    - gcc main.c -o main
    - gcc user.c -o user

2. Uruchamianie
    należy najpierw uruchomić serwer, a następnie zalogować dwóch użytkowników
    gdy dwaj użytkownicy zostaną wykryci przez serwer gra się rozpocznie
    - ./main - serwer
    - ./user (0 lub 1) - klient
    
3. Opis plików
    gameMemory.h - struktura pamięci współdzielonej + klucze 

    protocol.h - struktura kolejek komunikacyjnych

    main.c - serwer - obsługuje całą mechanikę gry, odbiera komendy od użytkonika i aktualizuje zasoby gracza

    user.c - klient - pozwala użytkonikowi zarządzanie swoją armią i surowcami za pomocą komend

4. Komendy opis
        kup - umożliwia zakup jednostek za surowce
        (następnie należy podać jedną z 4 możliwych jednostek)
            lpiechota - lekka piechota (c. 100, a. 1, o. 1.2, czas p. 2s)
            cpiechota - ciężka piechota (c. 250, a. 1.5, o. 3, czas p. 3s)
            jazda - jazda (c. 550, a. 3.5, o. 1.2, czas p. 5s) 
            robotnik - robotnik (c. 150, a. 0, o. 0, czas p. 2s) - każdy robotnik zwiększa produkcje surowców o 5 j. na sekundę
                (po podaniu jednostki należy podać ilość np. 4)

        atak - umożliwia atak na przeciwnika, wybraną ilością jednostek
        (następnie należy podać po kolei po zapytaniu o ilosc wysylanych jednostek liczbe - domyslnie 0)

        wyjscie - kończy program klienta

        help - wyświetla możliwe komendy 

5. Zasady gry 
    Każdy z graczy rozpoczyna z 300 jednostkami surowców na start, bez jednostek
    co 1 sekundę otrzymują oni po 50 jednostek + 5 za każdego robotnika
    Po 5 wygranych atakach gracz zwycięża i gra się kończy