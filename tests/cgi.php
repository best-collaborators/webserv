#!/usr/bin/env php
<?php
/**
 * Stateless PHP CGI Script
 * No cookies, no sessions
 * Handles:
 * - GET
 * - POST (urlencoded)
 * - multipart/form-data
 * - Streaming stdin
 * - JSON mode
 */

// ============================
// 1. ENVIRONMENT VARIABLES
// ============================

$METHOD         = $_SERVER['REQUEST_METHOD'] ?? 'GET';
$QUERY_STRING   = $_SERVER['QUERY_STRING'] ?? '';
$CONTENT_TYPE   = $_SERVER['CONTENT_TYPE'] ?? '';
$CONTENT_LENGTH = intval($_SERVER['CONTENT_LENGTH'] ?? 0);

$SCRIPT_NAME    = $_SERVER['SCRIPT_NAME'] ?? '';
$SERVER_NAME    = $_SERVER['SERVER_NAME'] ?? '';
$SERVER_PORT    = $_SERVER['SERVER_PORT'] ?? '';

error_log("CGI started: $METHOD $SCRIPT_NAME");

// ============================
// 2. HELPERS
// ============================

function send_headers($type = "text/html") {
    header("Content-Type: $type");
}

function escape_html($str) {
    return htmlspecialchars($str, ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8');
}

// ============================
// 3. PARSE GET
// ============================

$GET = $_GET;

// ============================
// 4. READ RAW STDIN (if needed)
// ============================

$rawBody = '';

if ($METHOD === 'POST') {
    $rawBody = file_get_contents("php://input");

    if (strlen($rawBody) > $CONTENT_LENGTH) {
        error_log("Payload exceeded declared size");
        exit(1);
    }
}

// ============================
// 5. PARSE POST
// ============================

function parse_post($method, $contentType, $rawBody) {

    if ($method !== 'POST') {
        return [];
    }

    // application/x-www-form-urlencoded
    if (str_starts_with($contentType, 'application/x-www-form-urlencoded')) {
        return $_POST;
    }

    // multipart/form-data
    if (str_starts_with($contentType, 'multipart/form-data')) {

        $result = [];

        // Normal fields
        foreach ($_POST as $key => $value) {
            $result[$key] = $value;
        }

        // Files
        foreach ($_FILES as $field => $file) {

            if ($file['error'] === UPLOAD_ERR_OK) {

                $safeName = basename($file['name']);
                $uploadPath = sys_get_temp_dir() . DIRECTORY_SEPARATOR . $safeName;

                move_uploaded_file($file['tmp_name'], $uploadPath);

                $result[$field] = [
                    'filename' => $safeName,
                    'savedTo'  => $uploadPath
                ];
            }
        }

        return $result;
    }

    // application/json
    if (str_starts_with($contentType, 'application/json')) {
        $decoded = json_decode($rawBody, true);
        return is_array($decoded) ? $decoded : [];
    }

    return [];
}

$POST = parse_post($METHOD, $CONTENT_TYPE, $rawBody);

// ============================
// 6. JSON MODE
// ============================

if (($GET['format'] ?? '') === 'json') {

    send_headers("application/json");

    echo json_encode([
        'env' => [
            'METHOD'      => $METHOD,
            'SERVER_NAME' => $SERVER_NAME,
            'SERVER_PORT' => $SERVER_PORT
        ],
        'GET'  => $GET,
        'POST' => $POST
    ], JSON_PRETTY_PRINT);

    exit;
}

// ============================
// 7. HTML OUTPUT
// ============================

send_headers("text/html");

echo "<html><body>";
echo "<h1>Stateless PHP CGI</h1>";

echo "<h2>Environment</h2><pre>";
echo escape_html(json_encode([
    'METHOD'      => $METHOD,
    'SCRIPT_NAME' => $SCRIPT_NAME,
    'SERVER_NAME' => $SERVER_NAME,
    'SERVER_PORT' => $SERVER_PORT
], JSON_PRETTY_PRINT));
echo "</pre>";

echo "<h2>GET</h2><pre>";
echo escape_html(json_encode($GET, JSON_PRETTY_PRINT));
echo "</pre>";

echo "<h2>POST</h2><pre>";
echo escape_html(json_encode($POST, JSON_PRETTY_PRINT));
echo "</pre>";

echo "</body></html>";
