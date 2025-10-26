<?php

use Saml2\Core\Config;

require_once 'Saml2/autoload.php';

$conf = new Config();
header('Content-Type: application/json');

# Do not allow this endpoint to be used if not in debug mode
if (!$conf->debug_mode) {
    http_response_code(403);
    echo json_encode(['error' => 'Forbidden']);
    exit();
}

# Require user to be logged in to access this endpoint
session_start();
if (!$_SESSION['Logged_In']) {
    http_response_code(401);
    echo json_encode(['error' => 'Unauthorized']);
    exit();
}

# Otherwise, return select session data
$data = [
    'saml2_user_data' => $_SESSION['saml2_user_data'] ?? [],
    'saml2_name_id' => $_SESSION['saml2_name_id'] ?? null,
    'username' => $_SESSION['Username'] ?? null,
    'allowed_pages' => $_SESSION['page-match'] ?? [],
];

echo json_encode($data);
