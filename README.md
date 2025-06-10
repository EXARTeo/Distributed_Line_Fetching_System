Γειά σας!  

Στον συγκεκριμένο φάκελο, περιλαμβάνονται τα εξής:

- Ένα απλό makefile (πατώντας `make`, κάνουμε compile όλα τα απαραίτητα προγράμματα, ενώ με `make clean` μπορούμε να διαγράψουμε τα αντικείμενα αρχεία που δημιουργήσαμε).
- Ένας φάκελος με την ονομασία **Modules**, στον οποίο βρίσκονται όλα τα προγράμματά μας **client.c**, **dispatcher.c** και **server.c**.
- Ένας φάκελος **Includes**, ο οποίος περιέχει τα κοινά headers μεταξύ των προγραμμάτων **client.h**, **dispatcher.h**.
- Ένα αρχείο **test.txt**, το οποίο αποτελεί το δοκιμαστικό αρχείο για τον dispatcher.

### Εκτέλεση και λειτουργίες των servers - dispatcher - client:

**Εκτέλεση:**  
Αρχικά, εκτελούμε την εντολή `make` για το compile των προγραμμάτων. Στη συνέχεια, τρέχουμε το πρόγραμμα dispatcher με την εντολή `./dispatcher FILENAME`, το οποίο θα εκκινήσει τον server. Σημειώνεται ότι αν τρέξουμε πρώτα κάποιο client, απλώς θα ενημερωθεί ότι ο server είναι κλειστός και θα τερματίσει. Μετά την εκκίνηση του dispatcher, μπορούμε να τρέχουμε το πρόγραμμα client όσες φορές θέλουμε – κάθε εκτέλεση `./client`, αντιστοιχεί σε έναν νέο client που συνδέεται με τον server μας.

Ο server - dispatcher περιμένει να λάβει input από κάποιον client. Μόλις δοθεί το input, εμφανίζεται στην κονσόλα είτε η ζητούμενη γραμμή από το αρχείο είτε ένα μήνυμα ότι η γραμμή δεν βρέθηκε, εάν:  
1. Το input ήταν μη θετικός ακέραιος (αν δοθεί θετικός αριθμός με δεκαδικά ψηφία, κρατάμε μόνο το ακέραιο μέρος και δεν θεωρείται λάθος).
2. Το αρχείο έχει λιγότερες γραμμές από αυτές που ζητά ο client.

Σε κάθε περίπτωση, εμφανίζεται ο χρόνος που χρειάστηκε ο client για να λάβει τα αποτελέσματα, από τη στιγμή που έδωσε το input. Μετά την ολοκλήρωση, ο επόμενος client που περίμενε στη σειρά λαμβάνει τη σκυτάλη για να δώσει το δικό του input στον dispatcher.  
Για να τερματίσει ένας client, πρέπει να δώσει ως input τη συμβολοσειρά `#exit#`. Μόλις το κάνει, δίνει τη σειρά του στον επόμενο και τερματίζει. Για να τερματίσουν όλα τα processes (server - dispatcher και όλοι οι clients), πρέπει κάποιος client να δώσει ως input τη συμβολοσειρά `#end#`. Σε αυτή την περίπτωση, ο client ενημερώνει τον server - dispatcher και τους υπόλοιπους clients να τερματίσουν.

Τέλος, αν θέλουμε να καθαρίσουμε τον χώρο από τα αντικείμενα αρχεία που δημιουργήθηκαν, τρέχουμε την εντολή `make clean`.

### Λειτουργίες των servers - dispatcher - client:

**Server:**  
Ο server εκτελεί τη βασική λειτουργία «εύρεση γραμμής» από το αρχείο .txt. Αποκτά πρόσβαση στη shared memory sm2, ανοίγει το αρχείο και το διαβάζει μέχρι να βρει την ζητούμενη γραμμή, την οποία στέλνει στον dispatcher μέσω της shared memory sm2, για να τη στείλει στον αντίστοιχο client. Μετά την ολοκλήρωση, επαναφέρει τον κέρσορα στην πρώτη γραμμή και περιμένει το επόμενο input από τον dispatcher. Αν για οποιονδήποτε λόγο το αρχείο δεν ανοίγει, ενημερώνει τον dispatcher για τερματισμό όλων των διεργασιών, ο οποίος θα πραγματοποιηθεί όταν προσπαθήσει να τρέξει ο πρώτος client.

**Dispatcher:**  
Ο dispatcher αρχικά δημιουργεί τις δύο shared memory περιοχές, sm1 (για τους clients και τον dispatcher) και sm2 (για τον server και τον dispatcher), και αρχικοποιεί τις αντίστοιχες μεταβλητές. Στη συνέχεια, με μια `fork()`, δημιουργεί ένα νέο process (το server process) και εκτελεί την εντολή `execlp()` για να αντικαταστήσει τη διεργασία του παιδιού με αυτή του server. Έτσι επιτυγχάνεται η έναρξη του server από τον dispatcher. Στο process του dispatcher (δηλαδή του γονέα), εκτελείται ο συντονισμός και η ενημέρωση των διεργασιών. Συγκεκριμένα:  
1) Αναμένει input από τον επόμενο client στη σειρά, για να ξεκινήσει η διαδικασία ομαλά.  
2) Ενημερώνει τους clients για τερματισμό σε περίπτωση σφάλματος του server, αλλά και τον server αν πρέπει να τερματίσει λόγω εντολής από κάποιο client.  
3) Αν κάποιος client θέλει να τερματίσει μόνο τον εαυτό του, ενημερώνει τον επόμενο στη σειρά να στείλει το δικό του input.  
4) Αν δεν υπάρξει διακοπή, στέλνει το input από τον client στον server και τον ενημερώνει να ξεκινήσει την «εύρεση γραμμής».  
5) Ενημερώνει τον client αν η γραμμή που ζήτησε βρέθηκε ή όχι και αν βρέθηκε, ενημερώνει για το ποιά είναι μέσο της sm1.  
6) Ενημερώνει τον επόμενο client να λάβει σειρά και αναμένει το επόμενο input.

**Client:**  
Ο client, αρχικά, ελέγχει αν υπάρχει ο dispatcher - server, ώστε να στείλει κάποιο μήνυμα (αν δεν υπάρχουν, τερματίζει εμφανίζοντας σχετικό μήνυμα). Ο client είναι υπεύθυνος για δύο συγχρονισμούς:  
1) Τη διαχείριση της σειράς των clients μεταξύ τους, ώστε να στέλνουν input στον dispatcher ένας-ένας.  
2) Τον συγχρονισμό του κάθε client με τον dispatcher.  
Αναλυτικά:  
1) Αν κάποιος client ήδη αλληλεπιδρά με τον dispatcher, ενημερώνει τους υπόλοιπους να περιμένουν.  
2) Αν ο server είναι κλειστός, τερματίζει, αφού πρώτα ενημερώσει τον επόμενο client να πράξει το ίδιο.  
3) Λαμβάνει το input από τον χρήστη (και σημειώνει την ώρα που το έλαβε). Αν το input είναι `#exit#`, ενημερώνει τον dispatcher να προχωρήσει με τον επόμενο client και τερματίζει. Αν είναι `#end#`, ενημερώνει τον dispatcher και τον server να τερματίσουν, και ακολουθεί η ίδια διαδικασία με το βήμα (2) για όλους τους clients.  
Αν δεν γίνει διακοπή, ο client δίνει το input στον dispatcher -> server, περιμένει τα αποτελέσματα, τα εκτυπώνει, σημειώνει την ώρα τερματισμού και εμφανίζει τον χρόνο που μεσολάβησε. Στη συνέχεια, ενημερώνει τον επόμενο client να ξεκινήσει.

Ακολουθεί ένα αναλυτικό παράδειγμα της εκτέλεσης των διεργασιών:
(Σημείωση: Για καλύτερη κατανόηση και διαχείριση του χώρου, τα τερματικά των clients εμφανίζουν μόνο τα τελευταία αποτελέσματα μετά από κάθε είσοδο.
Επιπλέον, η είσοδος των δεδομένων δεν γίνεται ταυτόχρονα. Κάθε client περιμένει να ολοκληρώσει τη διαδικασία ο client που προηγείται, προτού μπορέσει να καταχωρίσει την είσοδό του. Η είσοδος πραγματοποιείται διαδοχικά.)

```
---
Τερματικό 1                                 Τερματικό 2                                 Τερματικό 3
------------------------------------------------------------------------------------------------------
./dispatcher "test.txt"                     ./client                                    ./client
Server is starting...                       Client PID: 9000 waiting in queue           Client PID: 9050 waiting in queue
Server has started.                         The server is busy, please wait...
                                            Enter a line number to request from
                                            the server (or '#exit#' to quit or
                                            '#end#' to end everything):

------------------------------------------------------------------------------------------------------
./dispatcher "test.txt"                     ./client                                    ./client
Server is starting...                       Client PID: 9000 waiting in queue           Client PID: 9050 waiting in queue
Server has started.                         The server is busy, please wait...       
                                            Enter a line number to request from 
                                            the server (or '#exit#' to quit or
                                            '#end#' to end everything): 1

                                            The 1 line: This is the 1st line of the file.
                                            Microseconds needed to get the result: 569
------------------------------------------------------------------------------------------------------

./dispatcher "test.txt"                     ./client                                    ./client
Server is starting...                       Client PID: 9000 waiting in queue           Client PID: 9050 waiting in queue
Server has started.                                                                     The server is busy, please wait...
                                                                                        Enter a line number to request from 
Error: The file has fewer lines than                                                    the server (or '#exit#' to quit or
expected or the input was not positive                                                  '#end#' to end everything): AAA
integer.
                                                                                        Error: The file has fewer lines than 
                                                                                        expected or the input was not positive
                                                                                        integer. Microseconds needed to get 
                                                                                        the result: 571
------------------------------------------------------------------------------------------------------

./dispatcher "test.txt"                     ./client                                    ./client
Server is starting...                       Client PID: 9000 waiting in queue           Client PID: 9050 waiting in queue
Server has started.                         The server is busy, please wait...          The server is busy, please wait...
                                            Enter a line number to request from         Enter a line number to request from 
Error: The file has fewer lines than        the server (or '#exit#' to quit or          the server (or '#exit#' to quit or
expected or the input was not positive      '#end#' to end everything): 10              '#end#' to end everything): 7
integer.
                                            The 10 line: This is the 10th line          The 7 line: This is the 7th line
                                            of the file.                                of the file.
                                            Microseconds needed to get the result:      Microseconds needed to get the result:
                                            556                                         409
------------------------------------------------------------------------------------------------------
                                            (1ο Input)                                  (2o Input)

./dispatcher "test.txt"                     ./client                                    ./client
Server is starting...                       Client PID: 9000 waiting in queue           Client PID: 9050 waiting in queue
Server has started.                         The server is busy, please wait...          The server is busy, please wait...
                                            Enter a line number to request from         Enter a line number to request from 
Error: The file has fewer lines than        the server (or '#exit#' to quit or          the server (or '#exit#' to quit or
expected or the input was not positive      '#end#' to end everything): #exit#          '#end#' to end everything): #end#
integer.                                    $                                           $
$
------------------------------------------------------------------------------------------------------
```

Αυτή είναι μια σύντομη περιγραφή του τρόπου λειτουργίας και των λειτουργιών του προγράμματος. Ο κώδικας συνοδεύεται από αναλυτικά σχόλια για καλύτερη κατανόηση και η αλληλεπίδραση προγράμματος-χρήστη είναι απλή, διευκολύνοντας τις σχετικές δοκιμές. Τέλος και καλή σας συνέχεια!
