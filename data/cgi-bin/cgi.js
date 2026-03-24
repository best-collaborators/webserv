#!/usr/bin/env node

// ============================
// 0. PREVENT CGI BROKEN PIPE CRASH
// ============================

// If webserver closes pipe (client disconnect, timeout, invalid headers)
// exit silently instead of crashing with EPIPE
process.on('SIGPIPE', () => {
    process.exit(0);
});

process.stdout.on('error', (err) => {
    if (err.code === 'EPIPE') {
        process.exit(0);
    }
    throw err;
});

/**
 * Stateless Node.js CGI Script
 * No cookies, no sessions
 * Handles:
 * - GET
 * - POST (urlencoded)
 * - multipart/form-data
 * - Streaming stdin
 * - JSON mode
 */

const fs = require('fs');
const path = require('path');
const os = require('os');
const querystring = require('querystring');

// ============================
// 1. ENVIRONMENT VARIABLES
// ============================

const env = process.env;

const METHOD         = env.REQUEST_METHOD || 'GET';
const QUERY_STRING   = env.QUERY_STRING || '';
const CONTENT_TYPE   = env.CONTENT_TYPE || '';
const CONTENT_LENGTH = parseInt(env.CONTENT_LENGTH || '0', 10);

const SCRIPT_NAME    = env.SCRIPT_NAME || '';
const SERVER_NAME    = env.SERVER_NAME || '';
const SERVER_PORT    = env.SERVER_PORT || '';

// console.error("CGI started:", METHOD, SCRIPT_NAME);

// ============================
// 2. HELPERS
// ============================

function sendHeaders(type = "text/html") {
    process.stdout.write(`Content-Type: ${type}\r\n`);
    process.stdout.write("\r\n");
}

function escapeHTML(str) {
    return String(str).replace(/[&<>"]/g, c => ({
        '&': '&amp;',
        '<': '&lt;',
        '>': '&gt;',
        '"': '&quot;'
    })[c]);
}

// ============================
// 3. PARSE GET
// ============================

const GET = querystring.parse(QUERY_STRING);

// ============================
// 4. READ STDIN (STREAM SAFE)
// ============================

let body = '';

function readStdin(callback) {
    if (METHOD === 'POST' && CONTENT_LENGTH > 0) {

        let received = 0;

        process.stdin.on('data', chunk => {
            received += chunk.length;

            if (received > CONTENT_LENGTH) {
                console.error("Payload exceeded declared size");
                process.exit(1);
            }

            body += chunk;
        });

        process.stdin.on('end', callback);
    } else {
        callback();
    }
}

// ============================
// 5. PARSE POST
// ============================

function parsePost() {

    if (METHOD !== 'POST') return {};

    if (CONTENT_TYPE.startsWith('application/x-www-form-urlencoded')) {
        return querystring.parse(body);
    }

    if (CONTENT_TYPE.startsWith('multipart/form-data')) {
        return parseMultipart(body);
    }

    if (CONTENT_TYPE.startsWith('application/json')) {
        try {
            return JSON.parse(body);
        } catch {
            return {};
        }
    }

    return {};
}

// ============================
// 6. BASIC MULTIPART PARSER
// ============================

function parseMultipart(data) {

    const result = {};

    const boundaryMatch = CONTENT_TYPE.match(/boundary=(.+)$/);
    if (!boundaryMatch) return result;

    const boundary = boundaryMatch[1];
    const parts = data.split('--' + boundary);

    parts.forEach(part => {

        if (!part.includes('Content-Disposition')) return;

        const nameMatch = part.match(/name="([^"]+)"/);
        const fileMatch = part.match(/filename="([^"]*)"/);

        const splitIndex = part.indexOf('\r\n\r\n');
        if (splitIndex === -1) return;

        let value = part.substring(splitIndex + 4);
        value = value.replace(/\r\n--$/, '').trim();

        if (!nameMatch) return;

        const fieldName = nameMatch[1];

        if (fileMatch && fileMatch[1]) {

            const safeFilename = path.basename(fileMatch[1]);
            const uploadPath = path.join(os.tmpdir(), safeFilename);

            fs.writeFileSync(uploadPath, value);

            result[fieldName] = {
                filename: safeFilename,
                savedTo: uploadPath
            };

        } else {
            result[fieldName] = value;
        }
    });

    return result;
}

// ============================
// 7. MAIN EXECUTION
// ============================

readStdin(() => {

    const POST = parsePost();

    if (GET.format === 'json') {

        sendHeaders("application/json");

        process.stdout.write(JSON.stringify({
            env: {
                METHOD,
                SERVER_NAME,
                SERVER_PORT
            },
            GET,
            POST
        }, null, 2));

        return;
    }

    sendHeaders("text/html");

    process.stdout.write("<html><body>");
    process.stdout.write("<h1>Stateless Node CGI</h1>");

    process.stdout.write("<h2>Environment</h2><pre>");
    process.stdout.write(escapeHTML(JSON.stringify({
        METHOD,
        SCRIPT_NAME,
        SERVER_NAME,
        SERVER_PORT
    }, null, 2)));
    process.stdout.write("</pre>");

    process.stdout.write("<h2>GET</h2><pre>");
    process.stdout.write(escapeHTML(JSON.stringify(GET, null, 2)));
    process.stdout.write("</pre>");

    process.stdout.write("<h2>POST</h2><pre>");
    process.stdout.write(escapeHTML(JSON.stringify(POST, null, 2)));
    process.stdout.write("</pre>");

    process.stdout.write("<h2>BODY</h2><pre>");
    process.stdout.write(escapeHTML(body));
    process.stdout.write("</pre>");

    process.stdout.write("</body></html>");
});