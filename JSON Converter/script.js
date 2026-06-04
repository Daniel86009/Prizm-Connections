const fileInput = document.getElementById('fileInput');
const output = document.getElementById('output');

async function convert() {
    const file = fileInput.files[0];

    if (!file) {
        alert('Select json file first');
        return;
    }

    try {
        const text = await file.text();
        const data = JSON.parse(text);

        let out = 'ConnectionGame allGames[] = {</br>';
        let gameOuts = [];

        for (let i = 0; i < data.length; i++) {
            let game = data[i];

            gameOuts.push('{</br>{</br>');

            let groupOuts = [];
            for (let j = 0; j < game.answers.length; j++) {
                let temp = game.answers[j].group;
                temp = temp.replaceAll('"', '\\"');
                temp = temp.replaceAll('“', '\\"');
                temp = temp.replaceAll('”', '\\"');
                temp = temp.replaceAll('’', '\'');
                groupOuts.push('{"' + temp + '", ');
            }

            for (let j = 0; j < 4; j++) {
                let temp = '{';

                for (let l = 0; l < 4; l++) {
                    temp += '"' + game.answers[j].members[l] + '"';
                    if (l < 3) temp += ', ';
                }

                temp += '}}';

                groupOuts[j] += temp;
            }

            for (let j = 0; j < 4; j++) {
                gameOuts[i] += groupOuts[j];

                if (j < 3) gameOuts[i] += ',';
                gameOuts[i] += '</br>';
            }
            
            gameOuts[i] += '}</br>}';
        }

        for (let i = 0; i < gameOuts.length; i++) {
            out += gameOuts[i];
            if (i < gameOuts.length - 1) out += ',';
            out += '</br>';
        }

        out += '};';

        output.innerHTML = out;
    }
    catch (err) {
        console.error('Invalid JSON file: ', err);
    }
}