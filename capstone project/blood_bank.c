#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 200
#define GROUPS 8

const char *bloodGroups[GROUPS] = {"O-", "O+", "A-", "A+", "B-", "B+", "AB-", "AB+"};

struct Date { int day, month, year; };
struct BloodUnit { char group[4]; int quantity; struct Date collection, expiry; };
struct Donor { char name[60], group[4], phone[20]; struct Date lastDonation; };
struct Request { char hospital[60], patient[60], group[4]; int units; struct Date date; };

void readLine(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    if (fgets(buf, size, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
    }
}
int readInt(const char *prompt) {
    char line[100];
    int value;
    printf("%s", prompt);
    if (!fgets(line, sizeof(line), stdin)) return 0;
    while (sscanf(line, "%d", &value) != 1) {
        printf("Enter a valid number: ");
        if (!fgets(line, sizeof(line), stdin)) return 0;
    }
    return value;
}
void readDate(const char *prompt, struct Date *d) {
    printf("%s (DD MM YYYY): ", prompt);
    scanf("%d %d %d", &d->day, &d->month, &d->year);
    getchar();
}
int validGroup(const char *g) {
    for (int i = 0; i < GROUPS; i++) if (strcmp(g, bloodGroups[i]) == 0) return 1;
    return 0;
}
void readGroup(char *g) {
    do {
        printf("Blood group (O-, O+, A-, A+, B-, B+, AB-, AB+): ");
        scanf("%3s", g); getchar();
        if (!validGroup(g)) printf("Invalid blood group. Try again.\n");
    } while (!validGroup(g));
}
int dateCompare(struct Date a, struct Date b) {
    if (a.year != b.year) return a.year - b.year;
    if (a.month != b.month) return a.month - b.month;
    return a.day - b.day;
}
int loadStock(struct BloodUnit s[]) {
    FILE *fp = fopen("stock.dat", "rb");
    int n = 0;
    if (fp) { n = (int)fread(s, sizeof(struct BloodUnit), MAX, fp); fclose(fp); }
    return n;
}
void saveStock(struct BloodUnit s[], int n) {
    FILE *fp = fopen("stock.dat", "wb");
    if (!fp) { printf("Could not save stock file.\n"); return; }
    fwrite(s, sizeof(struct BloodUnit), n, fp); fclose(fp);
}
void addStock(void) {
    struct BloodUnit s[MAX]; int n = loadStock(s);
    if (n >= MAX) { printf("Stock list is full.\n"); return; }
    readGroup(s[n].group);
    s[n].quantity = readInt("Number of units to add: ");
    readDate("Collection date", &s[n].collection);
    readDate("Expiry date", &s[n].expiry);
    if (s[n].quantity <= 0 || dateCompare(s[n].expiry, s[n].collection) <= 0) {
        printf("Invalid quantity or expiry date. Record not saved.\n"); return;
    }
    n++; saveStock(s, n);
    printf("Stock added successfully.\n");
}
void viewStock(void) {
    struct BloodUnit s[MAX]; int n = loadStock(s), total[GROUPS] = {0};
    printf("\n--- BLOOD STOCK DETAILS ---\n");
    for (int i = 0; i < n; i++) {
        printf("%d) %s | Units: %d | Collected: %02d-%02d-%04d | Expires: %02d-%02d-%04d\n",
               i + 1, s[i].group, s[i].quantity, s[i].collection.day, s[i].collection.month, s[i].collection.year,
               s[i].expiry.day, s[i].expiry.month, s[i].expiry.year);
        for (int g = 0; g < GROUPS; g++) if (strcmp(s[i].group, bloodGroups[g]) == 0) total[g] += s[i].quantity;
    }
    printf("\n--- STOCK SUMMARY ---\n");
    for (int g = 0; g < GROUPS; g++) printf("%-3s : %d units\n", bloodGroups[g], total[g]);
    if (n == 0) printf("No stock records found.\n");
}
void nearExpiry(void) {
    struct BloodUnit s[MAX]; int n = loadStock(s);
    struct Date today;
    readDate("Enter today's date", &today);
    printf("\n--- NEAR-EXPIRY STOCK (within 7 days) ---\n");
    int found = 0;
    for (int i = 0; i < n; i++) {
        // Simple date window: compare expiry to today's date; this demonstration flags
        // expiry dates in the current month/year within 7 calendar days.
        if (s[i].expiry.year == today.year && s[i].expiry.month == today.month &&
            s[i].expiry.day >= today.day && s[i].expiry.day <= today.day + 7) {
            printf("%s | %d units | Expiry: %02d-%02d-%04d\n", s[i].group, s[i].quantity,
                   s[i].expiry.day, s[i].expiry.month, s[i].expiry.year); found = 1;
        }
    }
    if (!found) printf("No near-expiry stock found in the entered date window.\n");
}
int compatible(const char *patient, const char *donor) {
    // Red blood cell donor compatibility (simplified educational lookup)
    if (strcmp(patient, "O-") == 0) return strcmp(donor, "O-") == 0;
    if (strcmp(patient, "O+") == 0) return strcmp(donor, "O+") == 0 || strcmp(donor, "O-") == 0;
    if (strcmp(patient, "A-") == 0) return strcmp(donor, "A-") == 0 || strcmp(donor, "O-") == 0;
    if (strcmp(patient, "A+") == 0) return strcmp(donor, "A+") == 0 || strcmp(donor, "A-") == 0 || strcmp(donor, "O+") == 0 || strcmp(donor, "O-") == 0;
    if (strcmp(patient, "B-") == 0) return strcmp(donor, "B-") == 0 || strcmp(donor, "O-") == 0;
    if (strcmp(patient, "B+") == 0) return strcmp(donor, "B+") == 0 || strcmp(donor, "B-") == 0 || strcmp(donor, "O+") == 0 || strcmp(donor, "O-") == 0;
    if (strcmp(patient, "AB-") == 0) return strcmp(donor, "AB-") == 0 || strcmp(donor, "A-") == 0 || strcmp(donor, "B-") == 0 || strcmp(donor, "O-") == 0;
    if (strcmp(patient, "AB+") == 0) return 1;
    return 0;
}
void registerDonor(void) {
    struct Donor d;
    FILE *fp = fopen("donors.dat", "ab");
    if (!fp) { printf("Cannot open donor file.\n"); return; }
    readLine("Donor name: ", d.name, sizeof(d.name));
    readGroup(d.group);
    readLine("Phone number: ", d.phone, sizeof(d.phone));
    readDate("Last donation date", &d.lastDonation);
    fwrite(&d, sizeof(d), 1, fp); fclose(fp);
    printf("Donor registered.\n");
}
void searchDonors(void) {
    char patientGroup[4]; struct Donor d; int found = 0;
    readGroup(patientGroup);
    FILE *fp = fopen("donors.dat", "rb");
    if (!fp) { printf("No donor records yet.\n"); return; }
    printf("\nCompatible registered donors for %s (verify eligibility before contacting):\n", patientGroup);
    while (fread(&d, sizeof(d), 1, fp)) {
        if (compatible(patientGroup, d.group)) {
            printf("Name: %s | Group: %s | Phone: %s | Last donation: %02d-%02d-%04d\n",
                   d.name, d.group, d.phone, d.lastDonation.day, d.lastDonation.month, d.lastDonation.year);
            found = 1;
        }
    }
    fclose(fp); if (!found) printf("No matching donors found.\n");
}
void appendHistory(const char *message) {
    FILE *fp = fopen("history.dat", "a");
    if (fp) { fprintf(fp, "%s\n", message); fclose(fp); }
}
void recordRequest(void) {
    struct Request r; struct BloodUnit s[MAX]; int n = loadStock(s);
    readLine("Hospital name: ", r.hospital, sizeof(r.hospital));
    readLine("Patient name/ID: ", r.patient, sizeof(r.patient));
    readGroup(r.group);
    r.units = readInt("Units requested: ");
    readDate("Request date", &r.date);
    if (r.units <= 0) { printf("Invalid number of units.\n"); return; }
    FILE *req = fopen("requests.dat", "ab");
    if (req) { fwrite(&r, sizeof(r), 1, req); fclose(req); }
    // Issue only the exact requested group in this demonstration; staff must verify
    // clinical compatibility and cross-match before transfusion.
    int available = 0;
    for (int i = 0; i < n; i++) if (strcmp(s[i].group, r.group) == 0) available += s[i].quantity;
    if (available < r.units) {
        printf("Insufficient exact-group stock. Available: %d units. Request recorded but not issued.\n", available);
        char msg[300]; snprintf(msg, sizeof(msg), "REQUEST (NOT ISSUED): %s | Patient %s | %s | %d units | %02d-%02d-%04d", r.hospital, r.patient, r.group, r.units, r.date.day, r.date.month, r.date.year);
        appendHistory(msg); return;
    }
    // Sort stock batches of the requested group by earliest expiry date.
    for (int i = 0; i < n; i++) for (int j = i + 1; j < n; j++) {
        if (strcmp(s[i].group, r.group) == 0 && strcmp(s[j].group, r.group) == 0 && dateCompare(s[j].expiry, s[i].expiry) < 0) {
            struct BloodUnit temp = s[i]; s[i] = s[j]; s[j] = temp;
        }
    }
    int remaining = r.units;
    for (int i = 0; i < n && remaining > 0; i++) if (strcmp(s[i].group, r.group) == 0) {
        int take = s[i].quantity < remaining ? s[i].quantity : remaining;
        s[i].quantity -= take; remaining -= take;
    }
    // Remove empty batches
    int k = 0; for (int i = 0; i < n; i++) if (s[i].quantity > 0) s[k++] = s[i];
    saveStock(s, k);
    printf("Request recorded and stock updated (earliest expiry first).\n");
    char msg[300]; snprintf(msg, sizeof(msg), "ISSUED: %s | Patient %s | %s | %d units | %02d-%02d-%04d", r.hospital, r.patient, r.group, r.units, r.date.day, r.date.month, r.date.year);
    appendHistory(msg);
}
void viewHistory(void) {
    FILE *fp = fopen("history.dat", "r"); char line[400];
    if (!fp) { printf("No history records yet.\n"); return; }
    printf("\n--- REQUEST / ISSUE HISTORY ---\n");
    while (fgets(line, sizeof(line), fp)) printf("%s", line);
    fclose(fp);
}
void menu(void) {
    printf("\n========== BLOOD BANK SYSTEM =========\n");
    printf("1. Add Blood Stock\n2. View Stock Summary\n3. Check Near-Expiry Stock\n4. Register Donor\n5. Search Compatible Donors\n6. Record Hospital Request / Issue Stock\n7. View Request History\n0. Exit\n=====================================\n");
}
int main(void) {
    int choice;
    do {
        menu(); choice = readInt("Enter your choice: ");
        switch (choice) {
            case 1: addStock(); break;
            case 2: viewStock(); break;
            case 3: nearExpiry(); break;
            case 4: registerDonor(); break;
            case 5: searchDonors(); break;
            case 6: recordRequest(); break;
            case 7: viewHistory(); break;
            case 0: printf("Exiting Blood Bank System.\n"); break;
            default: printf("Invalid option. Please try again.\n");
        }
    } while (choice != 0);
    return 0;
}
