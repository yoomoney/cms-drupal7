<?php
/*
 * This file contains transaction operations.
 */

/**
 * Class YAMoneyApiTransaction
 */
class YAMoneyApiTransaction extends stdClass {
  const STATUS_IN_PROCESS = 'in_process';
  const STATUS_PROCESSED = 'processed';
  const STATUS_PAYED = 'payed';
  const STATUS_FAILED = 'failed';

  /**
   * @var int Transaction id.
   */
  public $ymid;
  /**
   * @var int Transaction user id.
   */
  public $uid;
  /**
   * @var int Transaction amount.
   */
  public $amount;
  /**
   * @var int Unix timestamp when the transaction was created.
   */
  public $created;
  /**
   * @var string Transaction status.
   */
  public $status;
  /**
   * @var string Transaction user email.
   */
  public $mail;
  /**
   * @var int Order id.
   */
  public $order_id;
  /**
   * @var array Array of additional transaction information.
   */
  public $data;

  public $payment_id;

  /**
   *
   */
  public function __construct() {
    $this->status = YAMoneyApiTransaction::STATUS_IN_PROCESS;
    $this->created = time();
    $this->data = array();
    $this->payment_id = '';
  }
}

/**
 * @param int $ymid Transaction id.
 * @return YAMoneyApiTransaction
 */
function yamoney_api_transaction_load($ymid) {
  $row = $transaction = db_select('yamoney_api_transaction', 'yt')
    ->fields('yt')
    ->condition('yt.ymid', $ymid)
    ->execute()
    ->fetch();

  if (!$row) {
    return NULL;
  }

  $transaction = new YAMoneyApiTransaction();
  foreach ($row as $key => $value) {
    $transaction->{$key} = $value;
  }

  $transaction->data = unserialize($transaction->data);

  return $transaction;
}

/**
 * @param int $order_id Order id.
 * @return YAMoneyApiTransaction
 */
function yamoney_api_transaction_load_by_order($order_id) {
    $row = $transaction = db_select('yamoney_api_transaction', 'yt')
        ->fields('yt')
        ->condition('yt.order_id', $order_id)
        ->execute()
        ->fetch();

    if (!$row) {
        return NULL;
    }

    $transaction = new YAMoneyApiTransaction();
    foreach ($row as $key => $value) {
        $transaction->{$key} = $value;
    }

    $transaction->data = unserialize($transaction->data);

    return $transaction;
}

/**
 * @param int $order_id Order id.
 * @return YAMoneyApiTransaction
 */
function yamoney_api_transaction_load_by_payment($payment_id) {
    $row = $transaction = db_select('yamoney_api_transaction', 'yt')
        ->fields('yt')
        ->condition('yt.payment_id',$payment_id)
        ->execute()
        ->fetch();

    if (!$row) {
        return NULL;
    }

    $transaction = new YAMoneyApiTransaction();
    foreach ($row as $key => $value) {
        $transaction->{$key} = $value;
    }

    $transaction->data = unserialize($transaction->data);

    return $transaction;
}

/**
 * @param YAMoneyApiTransaction $transaction
 * @return bool
 */
function yamoney_api_transaction_save(YAMoneyApiTransaction $transaction) {
  $fields = (array) $transaction;
  $fields['data'] = serialize($transaction->data);

  if ($transaction->ymid) {
    return !!db_update('yamoney_api_transaction')
      ->fields($fields)
      ->condition('ymid', $transaction->ymid)
      ->execute();
  }
  else {
    unset($fields['ymid']);
    $id = db_insert('yamoney_api_transaction')
      ->fields($fields)
      ->execute();
    $transaction->ymid = $id;
    return !!$id;
  }
}

/**
 * Simply set transaction status.
 * @param $ymid int Transaction id.
 * @param $status string Transaction status.
 * @return bool
 */
function yamoney_api_update_transaction_status($ymid, $status) {
  return !!db_update('yamoney_api_transaction')
    ->fields(array('status' => $status))
    ->condition('ymid', $ymid)
    ->execute();
}

/**
 * Delete user payment transaction
 * @param int $ymid
 * @return bool
 */
function yamoney_api_transaction_delete($ymid) {
  return !!db_delete('yamoney_api_transaction')
    ->condition('ymid', $ymid)
    ->execute();
}

/**
 * Render all transactions page
 * @return mixed
 */
function yamoney_api_transactions_page() {
  $transactions = db_select('yamoney_api_transaction', 'yt')
    ->fields('yt')
    ->extend('PagerDefault') // this adds the pager
    ->limit(20);
  $transactions = $transactions->execute()->fetchAll();

  $table = yamoney_api_create_transaction_table($transactions);
  $output = drupal_render($table);
  drupal_set_title(t('Transactions'));
  return $output;
}

/**
 * Render user transactions
 * @param stdClass $user
 * @return bool|string
 */
function yamoney_api_transactions_user_page($user) {
  $transactions = db_select('yamoney_api_transaction', 'yt')
    ->fields('yt')
    ->condition('uid', $user->uid)
    ->extend('PagerDefault') // this adds the pager
    ->limit(20);
  $transactions = $transactions->execute()->fetchAll();

  $table = yamoney_api_create_transaction_table($transactions);
  $output = drupal_render($table);
  drupal_set_title(t('My transactions'));
  return $output;
}


/**
 * @param array $table_rows
 * @return null|string
 */
function yamoney_api_create_transaction_table($table_rows) {
  $rows = array();
  if (count($table_rows) > 0) {
    foreach ($table_rows as $item) {
      $rows[] = yamoney_api_transaction_build_payment_row((array) $item);
    }
    $header = array(t('Id'), t('Name'), t('Amount'), t('Date/Time'), t('Status'), t('E-mail'), t('Order ID'));
    $table['table_rows'] = array(
      '#theme' => 'table',
      '#rows' => $rows,
      '#header' => $header,
    );

    $table['pager'] = array(
      '#theme' => 'pager',
      '#weight' => 5,
    );
  }
  else {
    $table = array(
      '#type' => 'markup',
      '#markup' => t('There no transactions.'),
    );
  }
  return $table;
}

/**
 * @param array $row
 * @return array
 */
function yamoney_api_transaction_build_payment_row($row) {
  unset($row['data']);
  foreach ($row as $key => $item) {
    if ($key == 'uid') {
      if ($item) {
        $user = user_load($item);
        $item = $user->name;
      }
      else {
        $item = 'guest';
      }
    }
    if ($key == 'created') {
      $item = format_date($item, 'custom', 'd-m-Y H:i:s');
    }
    if ($key == 'amount') {
      $item = $item . ' RUR';
    }
    $row[$key] = $item;
  }
  return $row;
}
