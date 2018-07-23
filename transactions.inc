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
 * @param string $payment_id
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
 * @param $payment_id string Transaction id in Kassa
 * @param $status string Transaction status.
 * @return bool
 */
function yamoney_api_update_transaction_status($payment_id, $status) {
  return !!db_update('yamoney_api_transaction')
    ->fields(array('status' => $status))
    ->condition('payment_id', $payment_id)
    ->execute();
}

/**
 * Simply set transaction status.
 * @param $payment_id string Transaction id in Kassa
 * @param $amount
 * @return bool
 */
function yamoney_api_update_transaction_amount($payment_id, $amount) {
    return !!db_update('yamoney_api_transaction')
        ->fields(array('amount' => $amount))
        ->condition('payment_id', $payment_id)
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
function yamoney_api_transactions_page()
{
    $transactions = db_select('yamoney_api_transaction', 'yt')
        ->fields('yt')
        ->extend('PagerDefault')// this adds the pager
        ->orderBy('ymid', 'DESC')
        ->limit(20);
    $transactions = $transactions->execute()->fetchAll();

    $table  = yamoney_api_create_transaction_table($transactions);
    $output = drupal_render($table);
    drupal_set_title(t('Transactions'));
    drupal_add_js('
        function yamoney_api_capture_payment_request() {
            document.getElementsByTagName("body")[0].style.cursor = "wait";
            let paymentId = this.dataset.paymentId;
            let r = new XMLHttpRequest();
            let el = document.createElement("span");
            el.innerText = this.text;
            this.parentElement.insertBefore(el, this);
            this.style.display = "none";
            r.open("POST", "'.url('admin/yamoney_api/actions').'", true);
            r.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
            r.onreadystatechange = function () {
                if (r.readyState !== 4) {
                    return;
                }
                document.getElementsByTagName("body")[0].style.cursor = "default";
                if(r.status === 200 && r.responseText === "OK") { 
                    alert("'.t('Вы подтвердили платёж в Яндекс.Кассе.').'");
                } else {
                    alert("'.t('Платёж не подтвердился. Попробуйте ещё раз.').'");
                }
                location.reload();
            };
            r.send("action=capturePayment&paymentId=" + this.dataset.paymentId 
                    + "&module=" + this.dataset.module);    
        }
        function yamoney_api_cancel_payment_request() {
            document.getElementsByTagName("body")[0].style.cursor = "wait";
            let paymentId = this.dataset.paymentId;
            let r = new XMLHttpRequest();
            let el = document.createElement("span");
            el.innerText = this.text;
            this.parentElement.insertBefore(el, this);
            this.style.display = "none";
            r.open("POST", "'.url('admin/yamoney_api/actions').'", true);
            r.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
            r.onreadystatechange = function () {
                if (r.readyState !== 4) {
                    return;
                }
                document.getElementsByTagName("body")[0].style.cursor = "default";
                if(r.status === 200 && r.responseText === "OK") {
                    alert("'.t('Вы отменили платёж в Яндекс.Кассе. Деньги вернутся клиенту.').'");
                } else {
                    alert("'.t('Платёж не отменился. Попробуйте ещё раз.').'");
                }
                location.reload();
            };
            r.send("action=cancelPayment&paymentId=" + this.dataset.paymentId
                    + "&module=" + this.dataset.module);    
        }
        document.addEventListener("DOMContentLoaded", function() {
           [].forEach.call(document.getElementsByClassName("yamoney_api_capture_request_link"), function(el) {
              el.addEventListener("click", yamoney_api_capture_payment_request);
           });
           [].forEach.call(document.getElementsByClassName("yamoney_api_cancel_request_link"), function(el) {
              el.addEventListener("click", yamoney_api_cancel_payment_request);
           });
        });', 'inline');
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
    ->orderBy('ymid', 'DESC')
    ->limit(20);
  $transactions = $transactions->execute()->fetchAll();

  $table = yamoney_api_create_transaction_table($transactions);
  $output = drupal_render($table);
  drupal_set_title(t('My transactions'));
  return $output;
}


/**
 * @param array $table_rows
 * @return array
 */
function yamoney_api_create_transaction_table($table_rows) {
  $rows = array();
  if (count($table_rows) > 0) {
    foreach ($table_rows as $item) {
      $row = yamoney_api_transaction_build_payment_row((array)$item);
      $orderId = $row['order_id'];
      $actions = '';
        $module = '';
        if (module_exists('commerce')
            && commerce_order_load($row['order_id'])
            && ya_commerce_api_commerce_payment_transaction_load($row['payment_id'])
        ) {
            $module          = 'commerce';
            $row['order_id'] = l($row['order_id'], 'admin/commerce/orders/'.$orderId);
            $actions         .= l(t('View'), 'admin/commerce/orders/'.$orderId).' ';
        } elseif (module_exists('uc_payment')
            && uc_order_load($row['order_id'])
        ) {
            $module          = 'uc_payment';
            $row['order_id'] = l($row['order_id'], 'admin/store/orders/'.$orderId);
            $actions         .= l(t('View'), 'admin/store/orders/'.$orderId).' ';
        }
        if ($row['status'] === \YandexCheckout\Model\PaymentStatus::WAITING_FOR_CAPTURE) {
            if ($module) {
                if ($module === 'commerce') {
                    $actions .= l(t('Edit'), 'admin/commerce/orders/'.$orderId.'/edit');
                } elseif ($module === 'uc_payment') {
                    $actions .= l(t('Edit'), 'admin/store/orders/'.$orderId.'/edit');
                }
                $actions .= ' <a href="javascript:void(0);" class="yamoney_api_capture_request_link" data-payment-id="'.$row['payment_id'].'" data-module="'.$module.'">'.t('Confirm').'</a> ';
                $actions .= '<a href="javascript:void(0);" class="yamoney_api_cancel_request_link" data-payment-id="'.$row['payment_id'].'" data-module="'.$module.'">'.t('Cancel').'</a>';
            }
        }
        $row[]  = $actions;
        $rows[] = $row;
    }
    $header = array(t('Id'), t('Name'), t('Amount'), t('Date/Time'), t('Status'), t('E-mail'), t('Order ID'), t('Payment ID'), t('Actions'));
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
