// the page's side of Hd/Web.h: unsaved changes, uploads, drops and
// downloads. linked into programs with the library (--pre-js).
if (!ENVIRONMENT_IS_PTHREAD) {
    // the program sets hdUnsaved, see Hd::Web::setUnsaved
    window.addEventListener('beforeunload', e => {
        if (Module.hdUnsaved) e.preventDefault();
    });

    // a file from the user's computer into the file system, written like
    // any other file (a persisted file system keeps it). the path once it's
    // written.
    const hdWrite = async (file, dir) => {
        const path = dir.replace(/\/$/, '') + '/' + file.name;
        FS.mkdirTree(dir);
        FS.writeFile(path, new Uint8Array(await file.arrayBuffer()));
        return path;
    };

    // files written by uploads and drops, taken by the program
    // (Hd::Web::takeUploaded)
    Module.hdUploaded = [];
    const hdWriteAll = async (files, dir) => {
        for (const f of files) {
            try {
                Module.hdUploaded.push(await hdWrite(f, dir));
            } catch (e) {
                console.error('uploading ' + f.name + ' failed', e);
            }
        }
    };

    Module.hdUpload = dir => {
        const input = document.createElement('input');
        input.type = 'file';
        input.multiple = true;
        input.addEventListener('change', () => hdWriteAll([...input.files], dir));
        input.click();
    };

    Module.hdDownload = path => {
        const url = URL.createObjectURL(new Blob([FS.readFile(path)]));
        const a = document.createElement('a');
        a.href = url;
        a.download = path.split('/').pop();
        a.click();
        setTimeout(() => URL.revokeObjectURL(url), 60000);
    };

    // files dropped on the page go into the folder the program sets
    Module.hdDropDir = '/home/web_user';
    window.addEventListener('dragover', e => {
        if (e.dataTransfer.types.includes('Files')) {
            e.preventDefault();
            e.dataTransfer.dropEffect = 'copy';
        }
    });
    window.addEventListener('drop', async e => {
        // the list empties once the handler returns, before the awaits
        const files = [...e.dataTransfer.files];
        if (!files.length) return;
        e.preventDefault();
        await hdWriteAll(files, Module.hdDropDir);
    });
}
