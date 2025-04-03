using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class Gan2_value_show : MonoBehaviour
{
    public Text gan2_value;
    public GameObject qudonggan2;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        string formatted = (qudonggan2.transform.eulerAngles.z-330.0f).ToString("F1");
        gan2_value.text = "孪生体控制值:" + formatted + "度";
    }
}
