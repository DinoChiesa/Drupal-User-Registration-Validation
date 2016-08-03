<?php
/**
 * @file
 * Supporting functions for Dpchiesa user registration validation.
 *
 * These functions are used in the .module and the .install
 */


function dpchiesa_validate_userreg_available_checks() {
  // A check returns true when the data is invalid.
  //
  // Today, checks are based on regex, and follow this pattern:
  //   name : the name of the check. Also the suffix of the name of the drupal variable that
  //          stores the enabled status of the check
  //   description : human-readable description of what the check does, presented to the admin.
  //   reason : human-readable description of the check violation, presented to the user when the check trips
  //   regex : a regular expression
  //   subject : the property in the $form_state['values'] array that the regex is compared against
  //
  // Other checks can call a function, which returns true when the check
  // should flag an account as invalid.
  //
  // A single check should not include both a regex and a function.

  $checks = array();
  $checks[] = array('name' => 'name_disallow_spaces',
                    'description' => 'disallow spaces in usernames',
                    'reason' => 'Spaces are not allowed in the username.',
                    'subject' => 'name',
                    'regex' => '/\s+/');
  $checks[] = array('name' => 'name_word_chars_only',
                    'description' => 'disallow non-word characters and spaces in usernames',
                    'reason' => 'Spaces and most non-word characters are disallowed in the username.',
                    'subject' => 'name',
                    'negate' => true,
                    'regex' => '/^\pL[\pL0-9_]+$/');
  $checks[] = array('name' => 'firstname_word_chars_only',
                    'description' => 'disallow non-word characters in firstnames',
                    'reason' => 'Non-word characters are not allowed in the firstname.',
                    'subject' => 'field_first_name',
                    'negate' => true,
                    'regex' => '/^\pL[\pL ]+$/'); // \pL means "word character" incl Unicode
  $checks[] = array('name' => 'lastname_word_chars_only',
                    'description' => 'disallow non-word characters in lastnames',
                    'reason' => 'Non-word characters are not allowed in the lastname.',
                    'subject' => 'field_last_name',
                    'negate' => true,
                    'regex' => '/^\pL[\pL ]+$/');

  // $domains = array('yahoo', 'googlemail', 'gmail', 'hotmail', 'live', 'outlook',
  //                  'mailinator', 'dispostable', 'maildrop', 'tafmail', '6paq',
  //                  'harakirimail', 'yopmail', '33mail');
  // $slug = implode('|', $domains);
  //
  //                   'regex' => '/@(' . $slug . ')\./');

  $checks[] = array('name' => 'email_domain_blacklist',
                    'description' => 'disallow specific email domains',
                    'reason' => 'invalid email address',
                    'subject' => 'mail',
                    'uncheck_when' => 'email_domain_whitelist',
                    'stringvar' => 'blacklisted_email_domains',
                    'stringdefault' => dpchiesa_validate_userreg_blacklist_default(),
                    'stringdescription' => 'a comma-separated list of email domains to block, eg "gmail, maildrop.com". If you use a string with no dot, it will block ALL domains.  Eg, use "gmail" to block gmail.co.uk, gmail.com, etc. Using spaces before or after the comma between entries is ok. Do not include quotes.',
                    'function' => 'dpchiesa_validate_userreg_evaluate_email_domain_blacklist');

  $checks[] = array('name' => 'email_domain_whitelist',
                    'description' => 'allow registration only from specific email domains',
                    'reason' => 'Email addresses from that domain are not approved for accounts here.',
                    'subject' => 'mail',
                    'uncheck_when' => 'email_domain_blacklist',
                    'stringvar' => 'whitelisted_email_domains',
                    'stringdefault' => dpchiesa_validate_userreg_drupal_hostname(),
                    'stringdescription' => 'a comma-separated list of email domains to whitelist, eg "example.org, apigee.com". Using spaces before or after the comma between entries is ok. Do not include quotes. Note: it does not make sense to use both blacklist and whitelist.',
                    'function' => 'dpchiesa_validate_userreg_evaluate_email_domain_whitelist');

  return $checks;
}


function dpchiesa_validate_userreg_form_value($form_values, $name) {
  // In some cases $form_values[$check['subject']] is returning an array.
  // This is because of Drupals auto-translation.
  // In those cases, dereference the language-neutral value.
  if (!isset($form_values[$name])) {
    return NULL;
  }
  $value = $form_values[$name];
  if (is_array($value)) {
    $value = (isset($value['und'][0]['value'])) ? $value['und'][0]['value'] : var_export($value, true);
  }
  return $value;
}


function dpchiesa_validate_userreg_evaluate_email_domain_whitelist($subject) {
  $prefix = 'dpchiesa_validate_userreg_';
  $varname = $prefix . 'whitelisted_email_domains';
  $domainlist = variable_get($varname, "apigee.com");
  $domains = array_map('preg_quote', array_map('trim', explode(',', $domainlist)));
  $slug = implode('|', $domains);
  $regex = '/@(' . $slug . ')$/';
  return !boolval(preg_match($regex, $subject)); // non-match = invalid email
}


function dpchiesa_validate_userreg_evaluate_email_domain_blacklist($subject) {
  $prefix = 'dpchiesa_validate_userreg_';
  $varname = $prefix . 'blacklisted_email_domains';
  $domainlist = variable_get($varname, "gmail.com");
  $domains = array_map('trim', explode(',', $domainlist));
  // some of the domains have dots, some do not.
  $regex_atoms = array_map(function ($d) {
      if (strpos($d, '.') === false) {
        return preg_quote($d . '.');
      }
      return preg_quote($d) . '$';
  }, $domains);
  $slug = implode('|', $regex_atoms);
  $regex = '/@(' . $slug . ')/';
  $result = boolval(preg_match($regex, $subject));
  return $result;  // positive match = invalid email
}


function dpchiesa_validate_userreg_apply_check($form_values, $check) {
  // A check returns true when the regex matches the subject.
  // TRUE implies invalid form data.
  //
  // Today, checks are regex based, or can call a function.
  // This function will need to get smarter if other kinds of checks are added.

  $subject = dpchiesa_validate_userreg_form_value($form_values, $check['subject']);
  if ($subject === NULL) {
    return FALSE;
  }

  $result = FALSE;
  if (isset($check['regex'])) {
    $result = boolval(preg_match($check['regex'], $subject));
  }
  elseif (isset($check['function'])) {
    $result = boolval(call_user_func($check['function'], $subject));
  }
  else {
    $result = "invalid rule";
  }

  if (isset($check['negate']) && $check['negate']) {
    $result = !boolval($result);
  }

  return $result;
}


function dpchiesa_validate_userreg_evaluate_check($form_values, $check, &$diagnostics) {
  // TRUE means invalid
  $prefix = 'dpchiesa_validate_userreg';
  $varname = $prefix . '_enabled_' . $check['name'];
  $check_enabled = variable_get($varname, 0);
  $result = ($check_enabled) ? dpchiesa_validate_userreg_apply_check($form_values, $check) : FALSE;

  $diagnostics[] = sprintf("%s enabled(%s) field(%s) subject(%s) result(%s)",
                           $check['name'],
                           $check_enabled,
                           $check['subject'],
                           dpchiesa_validate_userreg_form_value($form_values, $check['subject']),
                           var_export($result, true));
  return ($result === TRUE);
}


function dpchiesa_validate_userreg_evaluate_all_enabled_checks($form_values) {
  $prefix = 'dpchiesa_validate_userreg';
  $available_checks = dpchiesa_validate_userreg_available_checks();
  $diagnostics = array();
  foreach ($available_checks as $check) {
    if (dpchiesa_validate_userreg_evaluate_check($form_values, $check, $diagnostics)) {
      form_set_error($check['subject'], t($check['reason']));
    }
  }

  // optionally log this particular round of checks
  if (variable_get($prefix . '_logging', 0)) {
    watchdog('userreg_validation',
             "!message", array('!message' => implode(', ', $diagnostics)),
             WATCHDOG_NOTICE);
  }
}


function dpchiesa_validate_userreg_blacklist_default() {
  $domains = array('yahoo', 'googlemail', 'gmail', 'hotmail', 'live', 'outlook',
                  'mailinator', 'dispostable', 'maildrop', 'tafmail', '6paq',
                  'harakirimail', 'yopmail', '33mail');
  $slug = implode(', ', $domains);
  return $slug;
}

function dpchiesa_validate_userreg_drupal_hostname() {
  global $GLOBALS;
  // $GLOBALS['base_url'] refers to the root URL for the site, eg,
  //  == https://foo.bar.bam

  $hostname = $GLOBALS['base_url'];
  if (strpos($hostname, 'http://')==0) {
    $hostname = substr($hostname, 7);
  }
  elseif (strpos($hostname, 'https://')==0) {
    $hostname = substr($hostname, 8);
  }
  return $hostname;
}
