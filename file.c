// FAT16 512bytes/sector SFN read only

#define RDE_LEN			0x4000
#define LIM_USERCLUSTER	0xfff7

typedef struct {
	u32 fat, rde, len, ofs;
	u16 cluster, clustermask;
} File;

static File sFile;
static u32 sMediaAdr;

static void MediaSetAddress(u32 adr) {
	u32 sector = adr >> 9;
	u16 offset = adr & 0x1ff;
	sMediaAdr = adr;
	out(SECTOR0, ((u8 *)&sector)[3]);
	out(SECTOR1, ((u8 *)&sector)[2]);
	out(SECTOR2, ((u8 *)&sector)[1]);
	while (offset--) inp(STREAM);
}

static u8 MediaRead(void) {
	sMediaAdr++;
	return inp(STREAM);
}

static u16 MediaRead2(void) { // big endian
	u8 t[2];
	t[1] = MediaRead();
	t[0] = MediaRead();
	return *(u16 *)t;
}

static u32 MediaRead4(void) { // big endian
	u8 t[4];
	t[3] = MediaRead();
	t[2] = MediaRead();
	t[1] = MediaRead();
	t[0] = MediaRead();
	return *(u32 *)t;
}

void FileInit(void) {
	u32 w;
	File *f = &sFile;
	memset(f, 0, sizeof(File));
	MediaSetAddress(0x1c6); // first sector number
	w = MediaRead4() << 9;
	MediaSetAddress(w + 13);
	f->clustermask = (MediaRead() << 9) - 1;
	f->fat = w + 512;
	MediaRead4();
	MediaRead4();
	f->rde = f->fat + ((u32)MediaRead2() << 10);
}

static void FileSetCluster(u16 c) {
	if (c < LIM_USERCLUSTER) {
		File *f = &sFile;
		u32 adr = f->rde;
		if (c) adr += RDE_LEN + (u32)(c - 2) * (f->clustermask + 1);
		MediaSetAddress(adr);
	}
}

void FileOpen(u16 cluster, u32 len) {
	File *f = &sFile;
	FileSetCluster(f->cluster = cluster);
	f->len = len;
	f->ofs = 0;
}

int FileGetChar(void) {
	File *f = &sFile;
	if (f->ofs >= f->len) return -1; 
	if (f->ofs && !(f->ofs & f->clustermask)) {
		MediaSetAddress(f->fat + ((u32)f->cluster << 1));
		FileSetCluster(f->cluster = MediaRead2());
	}
	f->ofs++;
	return MediaRead();
}

void DirOpen(u16 cluster) {
	FileOpen(cluster, cluster ? 0 : RDE_LEN);
}

char *DirRead(void) {
	static char buf[32];
	File *f = &sFile;
	while (!f->len || f->ofs < f->len) {
		int i;
		for (i = 0; i < 32; i++) buf[i] = MediaRead();
		f->ofs += 32;
		if (!f->len && !(f->ofs & f->clustermask)) {
			MediaSetAddress(f->fat + ((u32)f->cluster << 1));
			f->cluster = MediaRead2();
			if (f->cluster >= LIM_USERCLUSTER) return NULL;
			FileSetCluster(f->cluster);
			f->ofs = 0;
		}
		switch (buf[0]) {
			case 0: case 5: case 0x2e: case 0xe5:
			case 0x5f: // resource fork of extracted tar
			break;
			default:
			if (buf[11] & 0xe) break;
			return buf;
		}
	}
	return NULL;
}

